/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_netxduo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "u_ethernet.h"
#include "u_messages.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
NX_PACKET_POOL NxAppPool;
NX_IP          NetXDuoEthIpInstance;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application NetXDuo Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

   /* USER CODE BEGIN App_NetXDuo_MEM_POOL */
  CHAR *pointer;
  /* USER CODE END App_NetXDuo_MEM_POOL */
  /* USER CODE BEGIN 0 */

  // u_TODO - the following code in this function was taken from the CubeMX-generated app_netxduo.c implementation.

  /* Initialize the NetXDuo system. */
  nx_system_initialize();

  /* Allocate the memory for packet pool. */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, NX_APP_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    printf("[app_netxduo.c/MX_NetXDuo_Init()] ERROR: Failed to allocate memory for packet pool.\n");
    return TX_POOL_ERROR;
  }

  /* Create the Packet pool to be used for packet allocation. */
  ret = nx_packet_pool_create(&NxAppPool, "NetXDuo App Pool", DEFAULT_PAYLOAD_SIZE, pointer, NX_APP_PACKET_POOL_SIZE);
  if(ret != NX_SUCCESS)
  {
    printf("[app_netxduo.c/MX_NetXDuo_Init()] ERROR: Failed to create packet pool (Status: %d).\n", ret);
    return NX_POOL_ERROR;
  }

  /* Allocate the memory for Ip_Instance */
  if(tx_byte_allocate(byte_pool, (VOID **) &pointer, Nx_IP_INSTANCE_THREAD_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    printf("[app_netxduo.c/MX_NetXDuo_Init()] ERROR: Failed to allocate memory for IP instance.\n");
    return TX_POOL_ERROR;
  }

  /* Create the main NX_IP instance */
  ret = nx_ip_create(&NetXDuoEthIpInstance, "NetX Ip instance", NX_APP_DEFAULT_IP_ADDRESS, NX_APP_DEFAULT_NET_MASK, pointer, Nx_IP_INSTANCE_THREAD_SIZE, NX_APP_INSTANCE_PRIORITY);
  if(ret != NX_SUCCESS)
  {
    printf("[app_netxduo.c/MX_NetXDuo_Init()] ERROR: Failed to create IP instance (Status: %d).\n", ret);
    return NX_NOT_SUCCESSFUL;
  }

  /* Allocate the memory for ARP */
  if(tx_byte_allocate(byte_pool, (VOID **) &pointer, DEFAULT_ARP_CACHE_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    printf("[app_netxduo.c/MX_NetXDuo_Init()] ERROR: Failed to allocate memory for ARP thread.\n");
    return TX_POOL_ERROR;
  }

  /* Enable the ARP protocol and provide the ARP cache size for the IP instance */
  ret = nx_arp_enable(&NetXDuoEthIpInstance, (VOID *)pointer, DEFAULT_ARP_CACHE_SIZE);
  if(ret != NX_SUCCESS)
  {
    printf("[app_netxduo.c/MX_NetXDuo_Init()] ERROR: Failed to enable ARP (Status: %d).\n", ret);
    return NX_NOT_SUCCESSFUL;
  }

  /* Enable the ICMP */
  ret = nx_icmp_enable(&NetXDuoEthIpInstance);
  if(ret != NX_SUCCESS)
  {
    printf("[app_netxduo.c/MX_NetXDuo_Init()] ERROR: Failed to enable ICMP (Status: %d).\n", ret);
    return NX_NOT_SUCCESSFUL;
  }

  /* Enable TCP Protocol */
  ret = nx_tcp_enable(&NetXDuoEthIpInstance);
  if(ret != NX_SUCCESS)
  {
    printf("[app_netxduo.c/MX_NetXDuo_Init()] ERROR: Failed to enable TCP (Status: %d).\n", ret);
    return NX_NOT_SUCCESSFUL;
  }

  /* Enable the UDP protocol required for DHCP communication */
  ret = nx_udp_enable(&NetXDuoEthIpInstance);
  if(ret != NX_SUCCESS)
  {
    printf("[app_netxduo.c/MX_NetXDuo_Init()] ERROR: Failed to enable UDP (Status: %d).\n", ret);
    return NX_NOT_SUCCESSFUL;
  }

  /* USER CODE END 0 */

  /* USER CODE BEGIN MX_NetXDuo_Init */
  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
