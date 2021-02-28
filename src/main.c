/**
 * test.c
 * Small Hello World! example
 * to compile with gcc, run the following command
 * gcc -o test test.c -lulfius
 */
#include <stdio.h>
#include <string.h>
#include <ulfius.h>
#include <stdbool.h>
#include <led.h>

const char *template = {
  "{"
  "\"led\": \"%s\""
  "}"
};

static LED_t led = {
        .gpio.pin = 0,
        .gpio.eMode = eModeOutput
    };

static char state[5] = "off";

char *LED_get_state(void)
{
  static char buffer[1024] = {0};
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, 1024, template, state);
  return buffer;
}

bool LED_set_state(const char *p_state)
{
  bool is_changed = false;
  if (strncasecmp(p_state, "on", 2) == 0)
  {
    memset(state, 0, sizeof(state));
    strncpy(state, p_state, 2);
    LED_set(&led, eStateHigh);
    is_changed = true;
  }
  else if (strncasecmp(p_state, "off", 3) == 0)
  {
    memset(state, 0, sizeof(state));
    strncpy(state, p_state, 3);
    LED_set(&led, eStateLow);
    is_changed = true;
  }

  return is_changed;
}

#define PORT 8095

int callback_index(const struct _u_request *request, struct _u_response *response, void *user_data)
{   
  ulfius_set_string_body_response(response, 200, LED_get_state());
  return U_CALLBACK_CONTINUE;
}

int callback_set_led(const struct _u_request *request, struct _u_response *response, void *user_data)
{ 
  char *p_state =  u_map_get(request->map_url, "state" );
  
  if(p_state && LED_set_state(p_state))
  {
    ulfius_set_string_body_response(response, 200, LED_get_state());
  }
  else 
  {
    ulfius_set_string_body_response(response, 404, "Resource not found");
  }

  
  return U_CALLBACK_CONTINUE;
}

int callback_default(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  ulfius_set_string_body_response(response, 404, "Page not found, do what you want");
  return U_CALLBACK_CONTINUE;
}

/**
 * main function
 */
int main(void)
{
  struct _u_instance instance;

   if(LED_init(&led))
        return EXIT_FAILURE;

  // Initialize instance with the port number
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK)
  {
    fprintf(stderr, "Error ulfius_init_instance, abort\n");
    return (1);
  }

  u_map_put(instance.default_headers, "Access-Control-Allow-Origin", "*");

  // Maximum body size sent by the client is 1 Kb
  instance.max_post_body_size = 1024;
  instance.max_post_param_size = 1024;

  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", "/led", NULL, 0, &callback_index, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", "/led/set", NULL, 0, &callback_set_led, NULL);
  

  ulfius_set_default_endpoint(&instance, &callback_default, NULL);

  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK)
  {
    printf("Start framework on port %d\n", instance.port);

    // Wait for the user to press <enter> on the console to quit the application
    getchar();
  }
  else
  {
    fprintf(stderr, "Error starting framework\n");
  }

  printf("End framework\n");
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);

  return 0;
}
