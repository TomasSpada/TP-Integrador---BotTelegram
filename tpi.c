//https://github.com/TomasSpada/TP-Integrador---BotTelegram

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <time.h>

struct memory {
  char *response;
  size_t size;
};

static size_t cb(char *data, size_t size, size_t nmemb, void *clientp)
{
  size_t realsize = nmemb;
  struct memory *mem = clientp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if(!ptr)
    return 0; 

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}

int main(void)
{

  long ultimo_update_id = 0;
  
  char *api_base = "https://api.telegram.org/bot";
  char token[50] = {0};
  char *cmd_getupdates = "/getUpdates"; 
  char *cmd_offset = "?offset=";
  char api_url[256];                                    
  
  FILE *tk = fopen("token.txt", "r");
  if (tk == NULL) {
	  printf("Error: no se pudo abrir token.txt\n");
	  return 1;
  } else {
	  fscanf(tk, "%49s", token); 
	  fclose(tk);
  }
  
  printf("Token cargado correctamente.\n");
  
  
  
  while(1){
	
	if (ultimo_update_id == 0) {
		  
		  snprintf(api_url, 256, "%s%s%s", api_base, token, cmd_getupdates);
	} else {
		  
		  snprintf(api_url, 256, "%s%s%s%s%ld",
				   api_base, token, cmd_getupdates, cmd_offset, ultimo_update_id + 1);
	}

	CURLcode res;
	CURL *curl = curl_easy_init();
	struct memory chunk = {0};

	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, api_url);
	
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		res = curl_easy_perform(curl);
		if (res != 0)
			printf("Error Codigo: %d\n", res);
	
		printf("%s\n", chunk.response);
		char *p = strstr(chunk.response, "\"update_id\"");
		if (p) {
			long nuevo_id = atol(p + 12);
			if (nuevo_id > ultimo_update_id) {
				ultimo_update_id = nuevo_id;
				printf("Nuevo update_id = %ld\n", nuevo_id);
			}
		}

		char *c = strstr(chunk.response, "\"chat\":{\"id\":");
		if (!c)
			c = strstr(chunk.response, "\"chat\": {\"id\":");
		
		long long chat_id = 0;
		
		if (c) {
			char *idpos = strstr(c, "\"id\":");
			if (idpos) {
				chat_id = atoll(idpos + 5);
			}
		}
		
		printf("chat_id = %lld\n", chat_id);
		
		char *t = strstr(chunk.response, "\"text\":\"");
		char texto[128] = {0};
		
		if (t) {
			t += 8; 
			int i = 0;
			while (t[i] != '"' && t[i] != 0 && i < 127) {
				texto[i] = t[i];
				i++;
			}
			texto[i] = 0;
			
			printf("Texto recibido: %s\n", texto);
			
			long long timestamp = 0;
			char first_name[128] = {0};
			
			char *d = strstr(chunk.response, "\"date\":");
			if (d) {
				timestamp = atoll(d + 7);  
			}
			
			char *f = strstr(chunk.response, "\"first_name\":\"");
			if (f) {
				f += 14;    
				int j = 0;
				while (f[j] != '"' && f[j] != 0 && j < 127) {
					first_name[j] = f[j];
					j++;
				}
				first_name[j] = 0;
			}
			
			
			if (texto[0] != 0) {
				FILE *logf = fopen("log.txt", "a");
				if (logf) {
					char fecha[64];
					time_t ts = (time_t)timestamp;
					struct tm *tm_info = localtime(&ts);
					strftime(fecha, sizeof(fecha), "%Y-%m-%d %H:%M:%S", tm_info);
					
					fprintf(logf, "%s | %s | %s\n", fecha, first_name, texto);
					
					fclose(logf);
				}
			}
			
		}
		
		char *d = strstr(chunk.response, "\"date\":");
		long long timestamp = 0;
		
		if (d) {
			timestamp = atoll(d + 7);
		}
		
		char *f = strstr(chunk.response, "\"first_name\":\"");
		char first_name[128] = {0};
		
		if (f) {
			f += 14;    
			int j = 0;
			while (f[j] != '"' && f[j] != 0 && j < 127) {
				first_name[j] = f[j];
				j++;
			}
			first_name[j] = 0;
			
			printf("Nombre del usuario: %s\n", first_name);
		}

		if (strcmp(texto, "hola") == 0 || strcmp(texto, "Hola") == 0) {
			
			char send_url[512];
			
			snprintf(send_url, 512,
					 "https://api.telegram.org/bot%s/sendMessage?chat_id=%lld&text=Hola%%20%s",
					 token, chat_id, first_name);
			
			printf("ENVIANDO SALUDO A: %s\n", first_name);
			
			CURL *curl2 = curl_easy_init();
			if (curl2) {
				curl_easy_setopt(curl2, CURLOPT_URL, send_url);
				curl_easy_setopt(curl2, CURLOPT_SSL_VERIFYPEER, 0L);
				curl_easy_setopt(curl2, CURLOPT_SSL_VERIFYHOST, 0L);
				
				curl_easy_perform(curl2);
				curl_easy_cleanup(curl2);
				FILE *log2 = fopen("log.txt", "a");
				if (log2) {
					fprintf(log2, "%lld | BOT | Hola %s\n", timestamp, first_name);
					fclose(log2);
				}
				
			}
		}

		if (strcmp(texto, "chau") == 0 || strcmp(texto, "Chau") == 0) {
			
			char send_url2[512];
			
			snprintf(send_url2, 512,
					 "https://api.telegram.org/bot%s/sendMessage?chat_id=%lld&text=Chau%%20%s",
					 token, chat_id, first_name);
			
			printf("ENVIANDO DESPEDIDA A: %s\n", first_name);
			
			CURL *curl3 = curl_easy_init();
			if (curl3) {
				curl_easy_setopt(curl3, CURLOPT_URL, send_url2);
				curl_easy_setopt(curl3, CURLOPT_SSL_VERIFYPEER, 0L);
				curl_easy_setopt(curl3, CURLOPT_SSL_VERIFYHOST, 0L);
				
				curl_easy_perform(curl3);
				curl_easy_cleanup(curl3);
				FILE *log3 = fopen("log.txt", "a");
				if (log3) {
					fprintf(log3, "%lld | BOT | Chau %s\n", timestamp, first_name);
					fclose(log3);
				}
				
			}
		}
		
		free(chunk.response);
		curl_easy_cleanup(curl);
   }
	sleep(2);
}
}
