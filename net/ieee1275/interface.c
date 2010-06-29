#include <grub/net/ieee1275/interface.h>
#include <grub/time.h>
#include <grub/mm.h>
#include <grub/net/ethernet.h>
#include <grub/net/ip.h>
#include <grub/net/arp.h>
#include <grub/net/netbuff.h>
#include <grub/ieee1275/ofnet.h>

static grub_ieee1275_ihandle_t handle;
int card_open (void)
{

  grub_ieee1275_open (grub_net->dev , &handle);
    return 0;

}

int card_close (void)
{

  if (handle)
    grub_ieee1275_close (handle);
  return 0;
}


int send_card_buffer (struct grub_net_buff *pack)
{
  
  int actual;
  grub_ieee1275_write (handle,pack->data,pack->tail - pack->data,&actual);
 
  return actual; 
}

int get_card_packet (struct grub_net_buff *pack __attribute__ ((unused)))
{

  int actual;
  char *datap; 
  struct iphdr *iph;
  struct etherhdr *eth;
  struct arphdr *arph;
  struct ip6hdr *ip6h;
  pack->data = pack->tail =  pack->head;
  datap = pack->data; 
  do
  {
    grub_ieee1275_read (handle,datap,sizeof (*eth),&actual);

  }while (actual <= 0);
  eth = (struct etherhdr *) datap;
  datap += sizeof(*eth);
 

  switch (eth->type)
  {
    case 0x806:

      grub_ieee1275_read (handle,datap,sizeof (*arph),&actual);
      arph = (struct arphdr *) datap;
      
      grub_netbuff_put (pack,sizeof (*eth) + sizeof (*arph));
    break;
    case 0x800:
      grub_ieee1275_read (handle,datap,sizeof (*iph),&actual);
      iph = (struct iphdr *) datap;
      datap += sizeof(*iph);

    
      grub_ieee1275_read (handle,datap,iph->len - sizeof (*iph),&actual);
       
    
      grub_netbuff_put (pack,sizeof (*eth) + iph->len);
    break;

    case 0x86DD:
      grub_ieee1275_read (handle,datap,sizeof (*ip6h),&actual);
      ip6h = (struct ip6hdr *) datap;

      datap += sizeof(*ip6h);
      grub_ieee1275_read (handle,datap,ip6h->payload_len - sizeof (*ip6h),&actual);
    break;

    default:
      grub_printf("Unknow packet %x\n",eth->type); 
    break;
  }
//  grub_printf("packsize %d\n",pack->tail - pack->data);
  return 0;// sizeof (eth) + iph.len; 
}
