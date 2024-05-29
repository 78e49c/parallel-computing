#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <pthread.h>

// İndirme işlemini yapacak fonksiyon
void *download_thread(void *arg) {
  char *url = (char *)arg;

  // CURL nesnesi oluşturma
  CURL *curl = curl_easy_init();

  // URL'yi ayarlama
  curl_easy_setopt(curl, CURLOPT_URL, url);

  // Dosya yazma fonksiyonu
  FILE *fp = fopen("downloaded_file", "wb");
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

  // İndirme işlemini başlatma
  curl_easy_perform(curl);

  // CURL nesnesini temizleme
  curl_easy_cleanup(curl);

  // Dosyayı kapatma
  fclose(fp);

  return NULL;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Kullanım: %s https://url.link thread_count\n", argv[0]);
    return 1;
  }

  // URL ve iş parçacığı sayısını alma
  char *url = argv[1];
  int thread_count = atoi(argv[2]);

  // İş parçacıkları dizisi
  pthread_t threads[thread_count];

  // İş parçacıklarını oluşturma
  for (int i = 0; i < thread_count; i++) {
    pthread_create(&threads[i], NULL, download_thread, url);
  }

  // İş parçacıklarının tamamlanmasını bekleme
  for (int i = 0; i < thread_count; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("Dosya başarıyla indirildi!\n");

  return 0;
}
