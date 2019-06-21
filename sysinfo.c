#include "sysinfo.h"

#include "stdlib.h"
#include "string.h"
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <stdio.h>

#define TEMP_BUFSIZE 8

double cpuPercent (){

  static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
  double percent;

  FILE* file;
  unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

  file = fopen("/proc/stat", "r");
  fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
         &totalSys, &totalIdle);
  fclose(file);

  if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
      totalSys < lastTotalSys || totalIdle < lastTotalIdle){
    //Overflow detection. Just skip this value.
    percent = -1.0;
  }
  else{
    total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
            (totalSys - lastTotalSys);
    percent = total;
    total += (totalIdle - lastTotalIdle);
    percent /= total;
    percent *= 100;
  }

  lastTotalUser = totalUser;
  lastTotalUserLow = totalUserLow;
  lastTotalSys = totalSys;
  lastTotalIdle = totalIdle;

  return percent;
}

double memPercent (){
  struct sysinfo memInfo;

  sysinfo (&memInfo);

  long long totalPhysMem = memInfo.totalram;
  totalPhysMem *= memInfo.mem_unit;

  long long physMemUsed = memInfo.totalram - memInfo.freeram;
  physMemUsed *= memInfo.mem_unit;

  long long physMemFree = totalPhysMem - physMemUsed;

  double percent = 100.0 * physMemUsed / totalPhysMem;
  //  printf("tot: %lld, used: %lld, free: %lld, per: %.2f\n", totalPhysMem, physMemUsed, physMemFree, percent);

  return percent;
}

const char* coreTemp (void) {
  char *cmd = "vcgencmd measure_temp";

  char *buf = malloc (sizeof (char) * TEMP_BUFSIZE);

//  char buf[TEMP_BUFSIZE];
  FILE *fp;

  if ((fp = popen(cmd, "r")) == NULL) {
    printf("Error opening pipe for coreTemp()!\n");
    exit(-1);
  }

  while (fgets(buf, TEMP_BUFSIZE, fp) != NULL) {
    // Do whatever you want here...
    printf("OUTPUT: %s", buf);
  }

  if(pclose(fp))  {
    printf("coreTemp(): Command not found or exited with error status\n");
    exit(-1);
  }

  return buf;
}

int main (){
  while (1){
    printf("cpu%%: %.2f\n", cpuPercent());
    printf("mem%%: %.2f\n", memPercent());
    printf("%s\n", coreTemp());
    sleep(1);
  }
}

