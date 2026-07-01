
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

sqlite3 *db;

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
     page = "<html>"
       "<body style='background-color: #0f172a; color: white; font-family: sans-serif; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0;'>"
       "  <div style='background-color: #1e293b; padding: 30px; border-radius: 15px; box-shadow: 0 10px 25px rgba(0,0,0,0.5); width: 350px; text-align: center;'>"
       "    <h2 style='color: #22d3ee; margin-bottom: 25px;'>Vulnerable Transit </h2>"
       "    <form method='POST' onsubmit='return validateForm()'>"
       "      <div style='text-align: left; margin-bottom: 15px;'>"
       "        <label style='display: block; font-size: 14px; margin-bottom: 5px; color: #94a3b8;'>Username</label>"
       "        <input type='text' name='username' id='uname' style='width: 100%; padding: 10px; border-radius: 8px; border: 1px solid #475569; background-color: #0f172a; color: white; outline: none; box-sizing: border-box;'>"
       "      </div>"
       "      <div style='text-align: left; margin-bottom: 25px;'>"
       "        <label style='display: block; font-size: 14px; margin-bottom: 5px; color: #94a3b8;'>Password</label>"
       "        <input type='password' name='password' id='upass' style='width: 100%; padding: 10px; border-radius: 8px; border: 1px solid #475569; background-color: #0f172a; color: white; outline: none; box-sizing: border-box;'>"
       "      </div>"
       "      <input type='submit' value='Register Account' style='width: 100%; padding: 12px; background: linear-gradient(to right, #06b6d4, #2563eb); border: none; border-radius: 8px; color: white; font-weight: bold; cursor: pointer; font-size: 16px;'>"
       "    </form>"
       "  </div>"
       "  <script>"
       "    function validateForm() {"
       "      var name = document.getElementById('uname').value;"
       "      var pass = document.getElementById('upass').value;"
       "      if (name == '' || pass == '') {"
       "        alert('Omo! You cannot leave fields blank!');"
       "        return false;"
       "      }"
       "      return true;"
       "    }"
       "  </script>"
       "</body></html>";
}
    else if (strcmp(method, "POST") == 0) {
      if (*req_cls == NULL) {
        static int marker = 1;
        *req_cls = &marker;
        return MHD_YES;
      }

      if (*upload_datasize > 0) {
	char extracted_name[50] = {0};
        char extracted_pass[50] = {0};
	 sscanf(upload_data, "username=%[^&]&password=%s", extracted_name, extracted_pass);

 int match_found = 0;

 for (int i = 0; i < user_count; i++) {
            
            if (strcmp(user_database[i].username, extracted_name) == 0 &&
                strcmp(user_database[i].password, extracted_pass) == 0) {
                
                match_found = 1; 
                break;           
            }
        }

 if (match_found == 1) {
            printf("Login Success for user: %s\n", extracted_name);
            
} else {
            printf("Login Failed for user: %s\n", extracted_name);
        }

    

   
        *upload_datasize = 0; 
        return MHD_YES; 
    }

        if (*upload_datasize == 0) {
            page = "<html><body><h1>Login Processing Complete!</h1></body></html>";
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


void db_init() {
    char *err_msg = NULL;

   
    int rc = sqlite3_open("vulnerabletransit.db", &db);
    if (rc != SQLITE_OK) {
        printf("Database open failed: %s\n", sqlite3_errmsg(db));
        return; 
    }

    
    sqlite3_exec(db, 
        "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, username TEXT, password TEXT);",
        NULL, NULL, &err_msg);

    sqlite3_exec(db, 
        "CREATE TABLE IF NOT EXISTS trips (id INTEGER PRIMARY KEY, destination TEXT, price INTEGER);",
        NULL, NULL, &err_msg);

sqlite3_exec(db, 
        "CREATE TABLE IF NOT EXISTS bookings (id INTEGER PRIMARY KEY, user_id INTEGER, trip_id INTEGER);",
        NULL, NULL, &err_msg);

    if (rc != SQLITE_OK && err_msg != NULL) {
        printf("SQL Table Creation Error: %s\n", err_msg);
        sqlite3_free(err_msg); 
    }
}

int main ()
{
    struct MHD_Daemon *daemon;
     db_init();

    
    daemon = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                               &answer_to_connection, NULL, MHD_OPTION_END);

    if (NULL == daemon) {
	sqlite3_close(db);		 
return 1;
}

  printf("Server is running on http://localhost:8888/ \n");
    printf("Press ENTER to stop the server...\n");

    getchar ();

    MHD_stop_daemon (daemon);
    sqlite3_close(db);
    return 0;
}



