#include "interfaces.h"
#include "../utils/errorutils.h"
#include <linux/if_packet.h>
#include <netinet/in.h>
#include <pcap.h>
#include <pcap/pcap.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>


typedef struct __InterfaceSpec
{
    char name[14];
    u_char mac[6];
    u_char ip[4];
    struct __InterfaceSpec *next;

} InterfaceSpec;

static void get_interface_addresses(pcap_addr_t *addressIterator,
                                    InterfaceSpec *target)
{
    struct sockaddr_in *ipAddress;
    struct sockaddr_ll *macAddress;
    while (addressIterator)
    {
        if (addressIterator->addr->sa_family == AF_PACKET)
        {
            macAddress = (struct sockaddr_ll *)(addressIterator->addr);
            memcpy(target->mac, macAddress->sll_addr, 6);
        }
        else if (addressIterator->addr->sa_family == AF_INET)
        {
            ipAddress = (struct sockaddr_in *)(addressIterator->addr);
            memcpy(target->ip, &ipAddress->sin_addr, 4);
        }
        addressIterator = addressIterator->next;
        if ((long)target->mac != 0l && (long)target->ip != 0l)
        {
            break;
        }
    }
}

static InterfaceSpec *get_all_interfaces()
{
    InterfaceSpec *root = NULL;
    InterfaceSpec *specIterator = root;
    InterfaceSpec *previous = NULL;

    static pcap_if_t *allDevices;
    static pcap_if_t *deviceIterator;

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_findalldevs(&allDevices, errbuf);
    deviceIterator = allDevices;

    while (deviceIterator)
    {
        specIterator = (InterfaceSpec *)calloc(1, sizeof(InterfaceSpec));
        if (previous)
        {
            previous->next = specIterator;
        }
        if (!root)
        {
            root = specIterator;
        }
        memset(specIterator->name, 0, 14);
        strncpy(specIterator->name, deviceIterator->name, 13);
        get_interface_addresses(deviceIterator->addresses, specIterator);
        deviceIterator = deviceIterator->next;
        previous = specIterator;
        specIterator = specIterator->next;
    }
    pcap_freealldevs(allDevices);
    return root;
}

static void clean_interface_list(InterfaceSpec *interfaceList)
{
    InterfaceSpec *specIterator;
    InterfaceSpec *previous = interfaceList;
    do
    {
        specIterator = previous->next;
        free(previous);
        previous = specIterator;
    } while (specIterator);
}

static int get_interface_by_regex(InterfaceSpec *result, char *regexString)
{
    InterfaceSpec *allInterfaces = get_all_interfaces();
    regex_t nameregex;
    regcomp(&nameregex, regexString, REG_EXTENDED);
    InterfaceSpec *iterator = allInterfaces;
    for (; iterator != NULL; iterator = iterator->next)
    {
        if (regexec(&nameregex, iterator->name, 0, NULL, 0) == 0)
        {
            break;
        }
    }
    if (iterator)
    {
        memcpy(result, iterator, sizeof(InterfaceSpec));
        result->next = NULL;
    }
    clean_interface_list(allInterfaces);
    return (iterator != NULL) ? CONDITION_SUCCEED : CONDITION_FAIL;
}

static void mac_to_bytes(char *mac_string, u_char *bytes)
{
    int count = 0;
    char error_string[] = "Mac address must be in the form  xx:xx:xx:xx:xx:xx";
    if (strlen(mac_string) != 17)
    {
        error_wrapper(CONDITION_FAIL,error_string,NULL);
    }
    char *token_string = (char *)malloc(17 + 1);
    memset(token_string, 0, 17 + 1);
    strncpy(token_string, mac_string, 17);

    char *token = strtok(token_string, ":");
    if (token == NULL)
    {
       error_wrapper(CONDITION_FAIL,error_string,NULL);
    }
    while (token != NULL)
    {
        bytes[count++] =
            (token[0] % 32 + 9) % 25 * 16 + (token[1] % 32 + 9) % 25;
        token = strtok(NULL, ":");
    }
    if (count != 6)
    {
        error_wrapper(CONDITION_FAIL,error_string,NULL);
    }
}

static pcap_t *build_capture_interface(InterfaceSpec *interface)
{
    pcap_t *handle =NULL;
    char errbuf[PCAP_ERRBUF_SIZE];
    handle = pcap_create(interface->name, errbuf);
    error_wrapper(handle!=NULL,errbuf,NULL);
    error_wrapper(pcap_set_snaplen(handle, 65535)==0,"Couldn't set snaplength",NULL);  
    error_wrapper(pcap_set_promisc(handle, 0)==0, "Couldn't set promiscuous",NULL);
    error_wrapper(pcap_set_immediate_mode(handle, 0)==0,"Couldn't set immediate mode",NULL); 
    error_wrapper(pcap_set_timeout(handle, 2)==0, "Couldn't set timeout",NULL);
    error_wrapper(pcap_setnonblock(handle, 1, errbuf)==0, errbuf ,NULL);
   
    return handle;
}

static void apply_capture_filter(pcap_t *handle, char *mac_string)
{
    struct bpf_program fp;
    char full_filter[48];
    memset(full_filter, 0, 48);
    char base_filter[] = "ether src ";
    memcpy(full_filter, base_filter, strlen(base_filter));
    char *copy_mac = &full_filter[strlen(base_filter)];
    memcpy(copy_mac, mac_string, strlen(mac_string));
    error_wrapper(pcap_compile(handle, &fp, full_filter, 1, PCAP_NETMASK_UNKNOWN)==0,pcap_geterr(handle),NULL); 
    error_wrapper(pcap_setfilter(handle, &fp)==0,"Could not apply filter to interface",NULL);
}


void setup_capture_spec(CaptureSpec *target, char *interface_regex,char *bind_mac)
{
    
    InterfaceSpec interface_result;
    error_wrapper(get_interface_by_regex(&interface_result, interface_regex), "Could not find interface",NULL);
    mac_to_bytes(bind_mac,target->dest_mac);
    memcpy(target->own_mac,interface_result.mac,6);
    target->capture_handle = build_capture_interface(&interface_result);
    error_wrapper(pcap_activate(target->capture_handle)==0,"Couldn't activate capture interface",NULL);
    apply_capture_filter(target->capture_handle,bind_mac);
}
