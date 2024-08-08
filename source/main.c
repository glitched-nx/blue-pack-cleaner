#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <sys/stat.h>
#include <unistd.h>

u32 __nx_applet_type = AppletType_None;

#define INNER_HEAP_SIZE 0x9000
size_t nx_inner_heap_size = INNER_HEAP_SIZE;
char nx_inner_heap[INNER_HEAP_SIZE];

/**
 * @brief Initialisiert den Heap für die Anwendung.
 */
void __libnx_initheap(void)
{
    void *addr = nx_inner_heap;
    size_t size = nx_inner_heap_size;

    extern char *fake_heap_start;
    extern char *fake_heap_end;

    fake_heap_start = (char *)addr;
    fake_heap_end = (char *)addr + size;
}

/**
 * @brief Initialisiert die Anwendung und die benötigten Services.
 * 
 * Diese Funktion wird beim Start der Anwendung aufgerufen und initialisiert
 * die Services für die Kommunikation mit dem System und das Dateisystem.
 */
void __attribute__((weak)) __appInit(void)
{
    Result rc;

    rc = smInitialize();
    if (R_FAILED(rc))
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    rc = fsInitialize();
    if (R_FAILED(rc))
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    fsdevMountSdmc();
}

void __attribute__((weak)) userAppExit(void);

/**
 * @brief Beendet die Anwendung und die initialisierten Services.
 * 
 * Diese Funktion wird beim Beenden der Anwendung aufgerufen und sorgt dafür,
 * dass alle initialisierten Services ordnungsgemäß beendet werden.
 */
void __attribute__((weak)) __appExit(void)
{
    fsdevUnmountAll();
    fsExit();
    smExit();
}

/**
 * @brief Überprüft, ob eine Datei existiert.
 * 
 * @param file Der Pfad zur Datei.
 * @return true, wenn die Datei existiert, andernfalls false.
 */
bool fileExists(const char *file)
{
    struct stat buf;
    return (stat(file, &buf) == 0);
}

/**
 * @brief Löscht eine Datei, wenn sie existiert.
 * 
 * @param path Der Pfad zur Datei.
 */
void deleteFile(char *path)
{
    if (fileExists(path))
    {
        remove(path);
    }
}

/**
 * @brief Löscht einen Ordner und alle darin enthaltenen Dateien.
 * 
 * @param path Der Pfad zum Ordner.
 */
void deleteFolder(char *path)
{
    struct dirent *de;
    DIR *dir = opendir(path);

    if (dir == NULL)
        return;

    while ((de = readdir(dir)) != NULL)
    {
        if (de->d_type == DT_REG)
        {
            char *fullpath;
            fullpath = malloc(strlen(path) + strlen(de->d_name) + 2);
            strcpy(fullpath, path);
            strcat(fullpath, "/");
            strcat(fullpath, de->d_name);

            deleteFile(fullpath);

            free(fullpath);
        }
    }

    closedir(dir);
    rmdir(path);
}

/**
 * @brief Der Haupteinstiegspunkt der Anwendung.
 * 
 * Diese Funktion wird beim Start der Anwendung aufgerufen und führt die
 * definierten Löschoperationen durch.
 * 
 * @param argc Die Anzahl der Argumente.
 * @param argv Das Array der Argumente.
 * @return int Der Rückgabewert der Anwendung.
 */
int main(int argc, char *argv[])
{
    // Alte Tesla-Dateien löschen
    // deleteFolder("sdmc:/atmosphere/contents/010000000007E51A/flags");
    // deleteFolder("sdmc:/atmosphere/contents/010000000007E51A");

    // Nichole Logo löschen
    // deleteFolder("sdmc:/atmosphere/exefs_patches/bootlogo");

    // Falschen DeepSea NRO-Namen löschen
    // deleteFile("sdmc:/switch/DeepSea-Updater/DeepSeaUpdater.nro");

    // Sigpatches bereinigen
    deleteFolder("sdmc:/switch/.packages/cleanup Sigpatches");
    deleteFolder("sdmc:/atmosphere/exefs_patches/es_patches");
    deleteFolder("sdmc:/atmosphere/exefs_patches/nfim_ctest");
    deleteFolder("sdmc:/atmosphere/kip_patches/fs_patches");
    deleteFolder("sdmc:/atmosphere/kip_patches/loader_patches");
    deleteFile("sdmc:/bootloader/patches.ini");

    // Sich selbst löschen
    deleteFolder("sdmc:/atmosphere/contents/010000000000DA7A/flags");
    deleteFolder("sdmc:/atmosphere/contents/010000000000DA7A");

    return 0;
}
