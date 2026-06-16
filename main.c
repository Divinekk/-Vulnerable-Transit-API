

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>

#define PORT 8888

enum MHD_Result answer_to_connection  (void *cls, 
                          struct MHD_Connection *connection,
                          const char *url, 
                          const char *method, 
                          const char *version,
                          const char *upload_data,
                          size_t *upload_datasize, 
                          void **req_cls)

{

const char  *page;

if ( strcmp(url, "/health") == 0 ) {
    page = "<html><body><h1>System is Healthy</h1></body></html>";
}
else if ( strcmp(url, "/trips") == 0 ) {
    page = "<html><body><h1>Trips Dashboard</h1></body></html>";
}
else if (strcmp(url, "/users") == 0) {
    if(strcmp(method, "GET") == 0) {
     page = "<html><body><h1>Register</h1><form method='POST'><input type='submit' value='Click to Submit'/></form></body></html>";

}
else if (strcmp(method, "POST") == 0) {
      if (*upload_datasize == 0) {
	return MHD_YES;

}
else {
	printf("Captured data stream");
	*upload_datasize = 0;
	page = "<html><body><h1>Registration Submitted</h1></body></html>";
      }
    
    }
}

struct MHD_Response *response;
int ret;
response = MHD_create_response_from_buffer (strlen (page), (
void *) page, MHD_RESPMEM_PERSISTENT);
ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
MHD_destroy_response (response);
return ret;

}

int main ()
{
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                               &answer_to_connection, NULL, MHD_OPTION_END);
    
    if (NULL == daemon) return 1;

    printf("Server is running on http://localhost:8888/ \n");
    printf("Press ENTER to stop the server...\n");

    getchar ();

    MHD_stop_daemon (daemon);
    return 0;



}
