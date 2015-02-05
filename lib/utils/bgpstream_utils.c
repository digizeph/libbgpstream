/*
 * bgpstream
 *
 * Chiara Orsini, CAIDA, UC San Diego
 * chiara@caida.org
 *
 * Copyright (C) 2014 The Regents of the University of California.
 *
 * This file is part of bgpstream.
 *
 * bgpstream is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bgpstream is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bgpstream.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>


#include "khash.h"
#include "utils.h"

#include "bgpstream_utils.h"

/** Print functions */

char *bl_print_ipv4_pfx(bl_ipv4_pfx_t* pfx)
{
  char buf[INET6_ADDRSTRLEN+4];
  char *p = buf;

  /* print the address */
  if(bgpstream_addr_ntop(buf, INET6_ADDRSTRLEN, &(pfx->address)) == NULL)
    {
      return NULL;
    }

  while(*p != '\0')
    p++;

  /* print the mask */
  snprintf(p, 4, "/%"PRIu8, pfx->mask_len);

  /* copy. blergh */
  return strdup(buf);
}

/** @todo merge into a single print func */
char *bl_print_ipv6_pfx(bl_ipv6_pfx_t* pfx)
{
  char buf[INET6_ADDRSTRLEN+4];
  char *p = buf;

  /* print the address */
  if(bgpstream_addr_ntop(buf, INET6_ADDRSTRLEN, &(pfx->address)) == NULL)
    {
      return NULL;
    }

  while(*p != '\0')
    p++;

  /* print the mask */
  snprintf(p, 4, "/%"PRIu8, pfx->mask_len);

  /* copy. blergh */
  return strdup(buf);
}

char *bl_print_pfx_storage(bl_pfx_storage_t* pfx)
{
  char buf[INET6_ADDRSTRLEN+4];
  char *p = buf;

  /* print the address */
  if(bgpstream_addr_ntop(buf, INET6_ADDRSTRLEN, &(pfx->address)) == NULL)
    {
      return NULL;
    }

  while(*p != '\0')
    p++;

  /* print the mask */
  snprintf(p, 4, "/%"PRIu8, pfx->mask_len);

  /* copy. blergh */
  return strdup(buf);
}

char *bl_print_as(bl_as_storage_t *as)
{
  if(as->type == BL_AS_NUMERIC)
    {
      char as_str[16];
      sprintf(as_str, "%"PRIu32, as->as_number);
      return strdup(as_str);
    }
  if(as->type == BL_AS_STRING)
    {
      return strdup(as->as_string);
    }
    return "";
}


char *bl_print_aspath(bl_aspath_storage_t *aspath)
{
    if(aspath->type == BL_AS_NUMERIC && aspath->hop_count > 0)
      {
	char *as_path_str = NULL;
	char as[10];
	int i;
	// assuming 10 char per as number
	as_path_str = (char *)malloc_zero(sizeof(char) * (aspath->hop_count * 10 + 1));
	as_path_str[0] = '\0';
	sprintf(as, "%"PRIu32, aspath->numeric_aspath[0]);
	strcat(as_path_str, as);	
	for(i = 1; i < aspath->hop_count; i++)
	  {
	    sprintf(as, " %"PRIu32, aspath->numeric_aspath[i]);
	    strcat(as_path_str, as);
	  }
	return as_path_str;
      }    
    if(aspath->type == BL_AS_STRING)
      {
	return strdup(aspath->str_aspath);	      
      }
    return strdup("");
}

/* as-path utility functions */

bl_as_storage_t bl_get_origin_as(bl_aspath_storage_t *aspath)
{
  bl_as_storage_t origin_as;
  origin_as.type = BL_AS_NUMERIC;
  origin_as.as_number = 0;
  char *path_copy;
  char *as_hop;
  if(aspath->hop_count > 0)
    {
      if(aspath->type == BL_AS_NUMERIC)
	{
	  origin_as.as_number = aspath->numeric_aspath[aspath->hop_count-1];	
	}
      if(aspath->type == BL_AS_STRING)
	{ 
	  origin_as.type = BL_AS_STRING;
	  origin_as.as_string = strdup(aspath->str_aspath);
	  path_copy = strdup(aspath->str_aspath);
	  while((as_hop = strsep(&path_copy, " ")) != NULL) {    
	    free(origin_as.as_string);
	    origin_as.as_string = strdup(as_hop);
	  }
	}
    }
  return origin_as;
}


bl_as_storage_t bl_copy_origin_as(bl_as_storage_t *as)
{
  bl_as_storage_t copy;
  copy.type = as->type;
  if(copy.type == BL_AS_NUMERIC)
    {
      copy.as_number = as->as_number;
    }
  if(copy.type == BL_AS_STRING)
    {
      copy.as_string = strdup(as->as_string);
    }
  return copy;
}

void bl_origin_as_freedynmem(bl_as_storage_t *as)
{
  if(as->type == BL_AS_STRING)
    {
      free(as->as_string);
      as->as_string = NULL;
      as->type = BL_AS_TYPE_UNKNOWN;
    }
}


bl_aspath_storage_t bl_copy_aspath(bl_aspath_storage_t *aspath)
{
  bl_aspath_storage_t copy;
  copy.type = aspath->type;
  copy.hop_count = aspath->hop_count;
  if(copy.type == BL_AS_NUMERIC && copy.hop_count > 0)
    {
      int i;
      copy.numeric_aspath = (uint32_t *)malloc(copy.hop_count * sizeof(uint32_t));
      for(i=0; i < copy.hop_count; i++)
	{
	  copy.numeric_aspath[i] = aspath->numeric_aspath[i];
	}
    }
  if(copy.type == BL_AS_STRING && copy.hop_count > 0)
    {
      copy.str_aspath = strdup(aspath->str_aspath);
    }  
  return copy;
}


void bl_aspath_freedynmem(bl_aspath_storage_t *aspath)
{
  if(aspath->type == BL_AS_STRING)
    {
      free(aspath->str_aspath);
      aspath->str_aspath = NULL;
      aspath->type = BL_AS_TYPE_UNKNOWN;
    }
  if(aspath->type == BL_AS_NUMERIC)
    {
      free(aspath->numeric_aspath);
      aspath->numeric_aspath = NULL;
      aspath->type = BL_AS_TYPE_UNKNOWN;
    }
}


/** prefixes */
khint64_t bl_pfx_storage_hash_func(bl_pfx_storage_t prefix)
{
  khint64_t h;
  uint64_t address = 0;
  unsigned char *s6 = NULL;
  if(prefix.address.version == BGPSTREAM_ADDR_VERSION_IPV4)
    {
      address = ntohl(prefix.address.ipv4.s_addr);
    }
  if(prefix.address.version == BGPSTREAM_ADDR_VERSION_IPV6)
    {
      s6 =  &(prefix.address.ipv6.s6_addr[0]);
      address = *((uint64_t *) s6);
      address = ntohll(address);
    }
  h = address | (uint64_t) prefix.mask_len;
  return __ac_Wang_hash(h);
}

int bl_pfx_storage_hash_equal(bl_pfx_storage_t prefix1,
			      bl_pfx_storage_t prefix2)
{
  if(prefix1.mask_len == prefix2.mask_len)
    {
      return bgpstream_addr_storage_equal(&prefix1.address, &prefix2.address);
    }
  return 0;
}

khint32_t bl_ipv4_pfx_hash_func(bl_ipv4_pfx_t prefix)
{
  // convert network byte order to host byte order
  // ipv4 32 bits number (in host order)
  // uint32_t address = ntohl(prefix.address.ipv4.s_addr);
  uint32_t address = prefix.address.ipv4.s_addr;  
  // embed the network mask length in the 32 bits
  khint32_t h = address | (uint32_t) prefix.mask_len;
  return __ac_Wang_hash(h);
}

int bl_ipv4_pfx_hash_equal(bl_ipv4_pfx_t prefix1, bl_ipv4_pfx_t prefix2)
{
  return ( (prefix1.address.ipv4.s_addr == prefix2.address.ipv4.s_addr) &&
	   (prefix1.mask_len == prefix2.mask_len));
}

khint64_t bl_ipv6_pfx_hash_func(bl_ipv6_pfx_t prefix)
{
  // ipv6 number - we take most significative 64 bits only (in host order)
  unsigned char *s6 =  &(prefix.address.ipv6.s6_addr[0]);
  uint64_t address = *((uint64_t *) s6);
  // address = ntohll(address);
  // embed the network mask length in the 64 bits
  khint64_t h = address | (uint64_t) prefix.mask_len;
  return __ac_Wang_hash(h);
}

int bl_ipv6_pfx_hash_equal(bl_ipv6_pfx_t prefix1, bl_ipv6_pfx_t prefix2)
{

  // implementation 1: faster when inserting different prefixes

#if 0
  if(prefix1.mask_len != prefix2.mask_len)
    {
      return 0;
    }
  int i;
  for(i=0; i< 16; i++)
    {
      if(prefix1.address.ipv6.s6_addr[i] != prefix2.address.ipv6.s6_addr[i])
	return 0;
    }
  return 1;
#endif  

  // implementation 2: faster when inserting a lot of equal prefixes 
  
  return ( prefix1.mask_len == prefix2.mask_len &&
	   (memcmp(&(prefix1.address.ipv6.s6_addr[0]), &(prefix2.address.ipv6.s6_addr[0]), sizeof(uint64_t)) == 0) &&
	   (memcmp(&(prefix1.address.ipv6.s6_addr[8]), &(prefix2.address.ipv6.s6_addr[8]), sizeof(uint64_t)) == 0)
	   );

}


/** as numbers */
khint32_t bl_as_storage_hash_func(bl_as_storage_t as)
{
  khint32_t h = 0;
  if(as.type == BL_AS_NUMERIC)
    {
      h = as.as_number;
    }
  if(as.type == BL_AS_STRING)
    {
      // if the string is at least 32 bits
      // then consider the first 32 bits as
      // the hash
      if(strlen(as.as_string) >= 4)
	{
	  h = * ((khint32_t *) as.as_string);
	}
      else
	{
	  // TODO: this could originate a lot of collisions
	  // otherwise 0
	  h = 0; 
	}
    }
  return __ac_Wang_hash(h);
}


int bl_as_storage_hash_equal(bl_as_storage_t as1, bl_as_storage_t as2)
{
  if(as1.type == BL_AS_NUMERIC && as2.type == BL_AS_NUMERIC)
    {
      return (as1.as_number == as2.as_number);
    }
  if(as1.type == BL_AS_STRING && as2.type == BL_AS_STRING)
    {
      return (strcmp(as1.as_string, as2.as_string) == 0);
    }
  return 0;
}


