#include <stdio.h>
#include <ulfius.h>
#include <jansson.h>
#include <sys/syscall.h>

#define PORT 9001

#define top_procs 5

struct proc_info {
    char name[16];
    long pid;
    unsigned long pct_usage;
    unsigned long mm_rss;
};

// Callback function for the endpoint /use_mem
int callback_use_mem (const struct _u_request * request, struct _u_response * response, void * user_data) {
    // Configure CORS
    u_map_put(response->map_header, "Access-Control-Allow-Origin", "*");
    // Get the memory usage
    unsigned long *_info = calloc(4, sizeof(unsigned long));
    long ret = syscall(548, _info);
    // Create a json object
    json_t *json;
    if (ret == 0) {
        json = json_pack("{s:f, s:f, s:f, s:f}", "free", ((double)_info[0])/1024,
         "used", ((double)_info[1])/1024, "cached", ((double)_info[2])/1024, "total", ((double)_info[3])/1024);
        ulfius_set_json_body_response(response, 200 , json);
    } else {
        json = json_pack("{s:s}", "error", "Error getting memory usage");
        ulfius_set_json_body_response(response, 500 , json);
    }
    free(_info);
    json_decref(json);
    return U_CALLBACK_CONTINUE;
}

// Callback function for the endpoint /use_swap
int callback_use_swap (const struct _u_request * request, struct _u_response * response, void * user_data) {
    // Configure CORS
    u_map_put(response->map_header, "Access-Control-Allow-Origin", "*");
    // Get the memory usage
    unsigned long *_info = calloc(4, sizeof(unsigned long));
    long ret = syscall(548, _info);
    double mem_used_fisic = ((double)_info[1])/1024; // Get the used memory
    // Get the swap usage
    ret = syscall(549, _info);
    double mem_used_swap = ((double)_info[0]*4)/1024; // Get the used memory
    // Create a json object
    json_t *json;
    if (ret == 0) {
        json = json_pack("{s:f, s:f}", "used_fisic", mem_used_fisic, "used_swap", mem_used_swap);
        ulfius_set_json_body_response(response, 200 , json);
    } else {
        json = json_pack("{s:s}", "error", "Error getting memory usage");
        ulfius_set_json_body_response(response, 500 , json);
    }
    free(_info);
    json_decref(json);
    return U_CALLBACK_CONTINUE;
}

// Callback function for the endpoint /fault_pages
int callback_fault_pages (const struct _u_request * request, struct _u_response * response, void * user_data) {
    // Configure CORS
    u_map_put(response->map_header, "Access-Control-Allow-Origin", "*");
    // Get the fault pages
    unsigned long *_info = calloc(2, sizeof(unsigned long));
    long ret = syscall(550, _info);
    // Create a json object
    json_t *json;
    if (ret == 0) {
        json = json_pack("{s:i, s:i}", "minor", _info[0], "major", _info[1]);
        ulfius_set_json_body_response(response, 200 , json);
    } else {
        json = json_pack("{s:s}", "error", "Error getting fault pages");
        ulfius_set_json_body_response(response, 500 , json);
    }
    free(_info);
    json_decref(json);
    return U_CALLBACK_CONTINUE;
}

// Callback function for the endpoint /use_pages
int callback_use_pages (const struct _u_request * request, struct _u_response * response, void * user_data) {
    // Configure CORS
    u_map_put(response->map_header, "Access-Control-Allow-Origin", "*");
    // Get the pages usage
    unsigned long *_info = calloc(2, sizeof(unsigned long));
    long ret = syscall(551, _info);
    // Create a json object
    json_t *json;
    if (ret == 0) {
        json = json_pack("{s:i, s:i}", "active", _info[0], "inactive", _info[1]);
        ulfius_set_json_body_response(response, 200 , json);
    } else {
        json = json_pack("{s:s}", "error", "Error getting pages usage");
        ulfius_set_json_body_response(response, 500 , json);
    }
    free(_info);
    json_decref(json);
    return U_CALLBACK_CONTINUE;
}

// Callback function for the endpoint /use_mem_top_procs
int callback_use_mem_top_procs (const struct _u_request * request, struct _u_response * response, void * user_data) {
    // Configure CORS
    u_map_put(response->map_header, "Access-Control-Allow-Origin", "*");
    // Get the top processes
    struct proc_info *procs_info = calloc(top_procs, sizeof(struct proc_info));
    long result = syscall(552, procs_info);
    if (result)
    {
        ulfius_set_json_body_response(response, 500, json_pack("{s:s}", "error", "Error getting top processes"));
        return U_CALLBACK_CONTINUE;
    }
    // Create a json array
    json_t *top_procs_array = json_array();
    if (top_procs_array == NULL)
    {
        ulfius_set_json_body_response(response, 500, json_pack("{s:s}", "error", "Error creating json array"));
        return U_CALLBACK_CONTINUE;
    }
    for (int i = 0; i < top_procs; i++)
    {
        json_t *proc_info = json_pack("{s:s, s:i, s:f, s:i}", "name", procs_info[i].name, "pid", procs_info[i].pid, "pct_usage", ((double)procs_info[i].pct_usage)/10, "mm_rss", procs_info[i].mm_rss);
        int ret = json_array_append_new(top_procs_array, proc_info);
        if (ret)
        {
            ulfius_set_json_body_response(response, 500, json_pack("{s:s}", "error", "Error appending json object to array"));
            return U_CALLBACK_CONTINUE;
        }
    }
    ulfius_set_json_body_response(response, 200, top_procs_array);
    free(procs_info);
    json_decref(top_procs_array);
    return U_CALLBACK_CONTINUE;
}

int main(void) {
    struct _u_instance instance;

    // Initialize instance with the port number
    if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error ulfius_init_instance, abort\n");
        return(1);
    }

    // Endpoint list declaration
    ulfius_add_endpoint_by_val(&instance, "GET", "/use_mem", NULL, 0, &callback_use_mem, NULL);
    ulfius_add_endpoint_by_val(&instance, "GET", "/use_swap", NULL, 0, &callback_use_swap, NULL);
    ulfius_add_endpoint_by_val(&instance, "GET", "/fault_pages", NULL, 0, &callback_fault_pages, NULL);
    ulfius_add_endpoint_by_val(&instance, "GET", "/use_pages", NULL, 0, &callback_use_pages, NULL);
    ulfius_add_endpoint_by_val(&instance, "GET", "/use_mem_top_procs", NULL, 0, &callback_use_mem_top_procs, NULL);
    

    // Start the framework
    if (ulfius_start_framework(&instance) == U_OK) {
        printf("Start framework on port %d\n", instance.port);

        // Wait for the user to press <enter> on the console to quit the application
        getchar();
    } else {
        fprintf(stderr, "Error starting framework\n");
    }
    printf("End framework\n");

    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return 0;
}