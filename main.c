#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>


#define PORT 8888

struct User {
        char username[50];
        char password[50];
};

struct User user_database[100];
int user_count =0;

void register_new_user(const char *upload_data) {
    const char *user_start;
    const char *ampersand_ptr;
    const char *pass_start;
    int user_length = 0;

    user_start = strstr(upload_data, "username=");
    if (user_start == NULL) return;

    user_start = user_start + 9;

    ampersand_ptr = strchr(user_start, '&');
    if (ampersand_ptr == NULL) return;

    user_length = ampersand_ptr - user_start;

    if (user_length > 49) {
        user_length = 49;
    }

    strncpy(user_database[user_count].username, user_start, user_length);
    user_database[user_count].username[user_length] = '\0';

    pass_start = strstr(upload_data, "password=");
    if (pass_start == NULL) return;

    pass_start = pass_start + 9;

    strncpy(user_database[user_count].password, pass_start, 49);
    user_database[user_count].password[49] = '\0';

    user_count++;
}

enum MHD_Result answer_to_connection  (void *cls,
                          struct MHD_Connection *connection,
                          const char *url,
                          const char *method,
                          const char *version,
                          const char *upload_data,
                          size_t *upload_datasize,
                          void **req_cls)

{

const char  *page = "<html><body><h1>404 Not Found</h1></body></html>";

if ( strcmp(url, "/health") == 0 ) {
    page = "<html><body><h1>System is Healthy</h1></body></html>";
}
else if ( strcmp(url, "/trips") == 0 ) {
    page = "<html><body><h1>Trips Dashboard</h1></body></html>";
}
else if (strcmp(url, "/users") == 0) {
    if(strcmp(method, "GET") == 0) {
     page = "<html><body><h1>Register</h1><form method='POST'>Username: <input type='text' name='username'><br>Password: <input type='password' name='password'><br><input type='submit' value='Click to Submit'/></form></body></html>";
    }
    else if (strcmp(method, "POST") == 0) {
      if (*req_cls == NULL) {
        static int marker = 1;
        *req_cls = &marker;
        return MHD_YES;
      }
      if (*upload_datasize == 0) {
        page = "<html><body><h1>Registration Submitted</h1></body></html>";
      }
      else {
        printf("Captured data stream");
        register_new_user(upload_data);
        *upload_datasize = 0;
        return MHD_YES;
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
