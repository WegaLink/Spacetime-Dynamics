
#include <DAVE.h>
#include "stdio.h"

#define IoTappVariables
#include "IoTapp.h"
#include "Html.h"

// dynamic client content
#define REL_DELAY 19
#define REL_HIGHMIN (REL_DELAY+14)
#define REL_HIGHMAX (REL_HIGHMIN+5)
#define REL_TRVMIN (REL_HIGHMAX+5)
#define REL_TRVMAX (REL_TRVMIN+4)

/* static IoT terminal headline */
#define http_iot_headline "IoTappXMC4500\r\n> "

static FATFS g_fatfs; /* File system object */
static FIL g_file1; /* File object */

/* web server thread function */
static void iot_server_netconn_thread(void *arg);

///* web server connection service threads */
//typedef void (*netconn_service_thread_t)(void *conn);
//typedef struct {
//	sys_thread_t NetconnServiceThreadID;
//	netconn_service_thread_t netconn_service_thread;
//
//} NetconnServiceThread_t;

/* netcon client */
static void iot_client_netconn_thread(void *arg);
#define REMOTE_PORT	41802

/** entry point to IoTapp functionality */
#define IOT_THREAD_STACKSIZE 1000
void runIoTapp()
{
	initDynamicWebContent();

	/* start a web server */
	NetconnServerThreadID_ = sys_thread_new("iot_server_netconn", iot_server_netconn_thread, NULL, IOT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
	/* start a netconn client */
	NetconnClientThreadID_ = sys_thread_new("iot_client_netconn", iot_client_netconn_thread, NULL, IOT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}

/** write to SD card */
static void
writeData(const char* Info, char* response, int length)
{
	do
	{
		/* mount file system */
		if(FR_OK != f_mount(&g_fatfs, "", 0))
			break;

		/* file open */
		if(FR_OK != f_open(&g_file1, "IoTapp.log", FA_OPEN_ALWAYS | FA_WRITE))
			break;

		/* move to end of the file to append data */
		if(FR_OK != f_lseek(&g_file1, f_size(&g_file1)))
			break;

		/* write message into file */
		UINT bw = 0; /* Bytes written */
		if(FR_OK != f_write(&g_file1, Info, strlen(Info), &bw))
			break;

		/* close file */
		if(FR_OK != f_close(&g_file1))
			break;

		/* unmount file system */
		if(FR_OK != f_mount(NULL, "", 0))
			break;

	} while (false);
}

/** IoT LED request */
#define LED2ON ": LED2 ON\r\n"
#define LED2OFF ": LED2 OFF\r\n"
static void
iot_led2_request(const char* cmd, char* response, int length)
{
	/* clear response */
	strcpy(response, "");

	/* evaluate params */
	if((strlen(cmd) < 6) || (length < strlen(LED2OFF)+1))
		return;

	/* process command */
	if(cmd[5] == '0'){
		XMC_GPIO_SetOutputLow(LED2_PIN);
		strcpy(response, LED2OFF);
		writeData(LED2OFF,response,length);
	}else if(cmd[5] == '1'){
		XMC_GPIO_SetOutputHigh(LED2_PIN);
		strcpy(response, LED2ON);
		writeData(LED2ON,response,length);
	}
}

/** IoT SD request */
static void
iot_sd_request(const char* cmd, char* response, int length)
{
	/* evaluate params */
	if(strcmp(cmd,"SD") != 0)
		return;

	do
	{
		/* mount file system */
		if(FR_OK != f_mount(&g_fatfs, "", 0))
			break;

		/* open file */
		if(FR_OK != f_open(&g_file1, "IoTapp.log", FA_OPEN_ALWAYS | FA_READ))
			break;

		/* move to one buffer length before end of file */
		int pos = f_size(&g_file1) - length + 1;
		if(FR_OK != f_lseek(&g_file1, pos >= 0 ? pos : 0))
			break;

		/* read data from file */
		UINT br = 0; /* Bytes read */
		if(FR_OK != f_read(&g_file1, response, length-1, &br))
			break;

		/* close file */
		if(FR_OK != f_close(&g_file1))
			break;

		/* unmount file system */
		if(FR_OK != f_mount(NULL, "", 0))
			break;

	} while (false);
}

/** Serve IoT request */
static void
iot_request_serve(struct netconn *conn)
{
  char response[80] = http_iot_headline;
  struct netbuf *inbuf;
  u16_t buflen;
  char *buf;

  do{
	/* send back response to IoT control terminal */
	if(ERR_OK != netconn_write(conn, response, strlen(response), NETCONN_NOCOPY))
		break;
	strcpy(response, "> ");

	/* get next IoT request */
	if(ERR_OK != netconn_recv(conn, &inbuf))
		break;
	if(ERR_OK != netbuf_data(inbuf, (void**)&buf, &buflen))
		break;
    /* Delete the buffer */
    netbuf_delete(inbuf);

	/* identify IoT request */
	if(buflen >= 4 && ((0 == strncmp(buf,"exit",4)) || (0 == strncmp(buf,"EXIT",4))))
		break;
	else if(buflen >= 4 && 0 == strncmp(buf,"LED2",4))
		iot_led2_request(buf, response, sizeof(response));
	else if(buflen >= 2 && 0 == strncmp(buf,"SD",2))
		iot_sd_request(buf, response, sizeof(response));
	else if(0 != strcmp(buf,"\r\n"))
		strcpy(response, "");
  }
  while(1U);
}

/** Serve one HTTP connection accepted in the web server thread */
static void
iot_server_netconn_serve(void *_conn)
{
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;
  struct netconn *conn = (struct netconn *)_conn;

  // thread bookkeeping
  Threads_++;

	/* Read the data from the port. We assume the request is in one netbuf */
	err = netconn_recv(conn, &inbuf);

	if (err == ERR_OK) {
		netbuf_data(inbuf, (void**)&buf, &buflen);

		/* Is this an HTTP GET command? */
		if (buflen>=5 && buf[0]=='G' && buf[1]=='E' && buf[2]=='T' && buf[3]==' ' && buf[4]=='/' ) {

			/* generate and send a html response */
			sendHtmlResponse(conn);
		} else{

			/* serve IoT request */
			iot_request_serve(conn);
		}
	}
	/* Delete the buffer */
	netbuf_delete(inbuf);

  /* Close the connection */
  netconn_shutdown(conn,0,1);
  osDelay(10);
  netconn_close(conn);
  netconn_delete(conn);

  // thread bookkeeping
  Threads_--;
}

static void
iot_server_netconn_service_thread(struct netconn *conn)
{
	// start netcon service thread
	sys_thread_t NetconnServiceThreadID = sys_thread_new("netconn_service_thread", &iot_server_netconn_serve, conn, IOT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

	// verify thread
	if(NetconnServiceThreadID <= 0){
		  netconn_close(conn);
		  netconn_shutdown(conn,true,true);
		  netconn_delete(conn);
	}
}

static void
iot_server_netconn_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  /* Create a new TCP server connection handle */
  conn = netconn_new(NETCONN_TCP);

  /* Bind to server port 80 (HTTP) with default IP address */
  if(ERR_OK != (err = netconn_bind(conn, NULL, 80))){
	  netconn_delete(conn);
	  return;
  }

  do {
	  /* Put the connection into LISTEN state */
	  if(ERR_OK != (err = netconn_listen(conn))){
		  netconn_delete(conn);
		  break;
	  }

	  do {
		/* wait endless for incoming connections */
		err = netconn_accept(conn, &newconn);
		if (err == ERR_OK) {
		  /* serve connection */
		  osDelay(10);
		  iot_server_netconn_service_thread(newconn);
		}
	  } while(err == ERR_OK);

  // workaround for accept() returning ERR_ABRT
  } while(true);

  /* fatal error on listening connection -> shutdown */
  netconn_shutdown(conn,1,1);
  netconn_delete(conn);
}

void iot_client_netconn_thread(void *arg)
{
	LWIP_UNUSED_ARG(arg);

	int rc1 = 0, rc2 = 0;
	struct netconn *xNetConn = NULL;
	ip_addr_t local_ip;
	ip_addr_t remote_ip;

	while(1){

		//                                        even__
		// NOTE: First MAC digit has to be EVEN! e.g. 00:03:19:45:00:51
		//       Otherwise it would be a group address.

		/* Create a new client TCP connection handle */
		xNetConn = netconn_new ( NETCONN_TCP );

		/* check TCP connection handle */
		if(NULL == xNetConn) return;

		/* Bind to ANY port at local IP */
		IP4_ADDR(&local_ip, 192, 168, NETWORK_IP, DEVICE_ID);
		rc1 = netconn_bind(xNetConn, &local_ip, 0);

		if(LAN_TEST){
			// TEST: connect to local SYSLOG server
			IP4_ADDR(&remote_ip, 192, 168, 42, 121);
			rc2 = netconn_connect ( xNetConn, &remote_ip, 514 );
		}else{
			// get remote IP address
			if(ERR_OK != netconn_gethostbyname("wegalink.feste-ip.net", &remote_ip)){
				/* use fix IP in case of any DNS problems */
				IP4_ADDR(&remote_ip, 185, 248, 148, 12);
			}
			// connect to remote SYSLOG server
			rc2 = netconn_connect ( xNetConn, &remote_ip, REMOTE_PORT );
		}

		/* check connection */
		if ( rc1 != ERR_OK || rc2 != ERR_OK ){
			// connection closed (or lost)
			netconn_close(xNetConn);
			netconn_delete(xNetConn);
			osDelay(CYCLE_MESSAGE_FTL*1000);
			continue;
		}

		while(1){

			// wait for next cycle start
			osSignalWait(SIGNAL_MESSAGE_FTL, osWaitForever);

			if(isCalibration){
				// send calibration message
				if( false == sendTimeflowCalibrationMessage(xNetConn)){
					// connection closed (or lost)
					netconn_close(xNetConn);
					netconn_delete(xNetConn);
					break;
				}
				isCalibration = false;
			}else{
				if(TELEMETRY){
					// send telemetry message
					if( false == sendTimeflowSolarPowerMessage(xNetConn)){
						// connection closed (or lost)
						netconn_close(xNetConn);
						netconn_delete(xNetConn);
						break;
					}
				}

				if(isDelayLocked_ || PING_TEST || (DEVICE_ID != 50)){
					// send timeflow ping
					if( false == sendTimeflowPing(xNetConn)){
						// connection closed (or lost)
						netconn_close(xNetConn);
						netconn_delete(xNetConn);
						break;
					}
				}
			}
		}
	}

	return;
}

