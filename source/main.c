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
    // Bootlogo bereinigen
    // deleteFolder("sdmc:/atmosphere/exefs_patches/bootlogo");

    // AIO Updater bereinigen
    // deleteFile("sdmc:/switch/aio-switch-updater/aio-switch-updater.nro");
    // deleteFolder("sdmc:/switch/aio-switch-updater");
    // deleteFolder("sdmc:/config/aio-switch-updater");

    // Theme deaktivieren, falls vorhanden 
    // deleteFile("sdmc:/atmosphere/contents/0100000000001000/flags/boot2.flag");
    // deleteFile("sdmc:/atmosphere/contents/0100000000001007/flags/boot2.flag");
    // deleteFile("sdmc:/atmosphere/contents/0100000000001013/flags/boot2.flag");

    // Hekate Configs etc. bereinigen
    deleteFile("sdmc:/bootloader/ini/more_conigs_core.ini");
    deleteFile("sdmc:/bootloader/ini/more_conigs_erista.ini");
    deleteFile("sdmc:/bootloader/ini/Stock.ini");
    deleteFile("sdmc:/bootloader/ini/Tegra.ini");
    deleteFile("sdmc:/bootloader/ini/hekate.ini");
    deleteFile("sdmc:/bootloader/ini/enigma.ini");
    deleteFile("sdmc:/bootloader/payloads/Hekate.bin");
    deleteFile("sdmc:/bootloader/boot/cfw_pack_bootlogo.bmp");
    deleteFile("sdmc:/bootloader/res/icon_pure_atmo_ofw_nobox.bmp");
    
    // Sigpatches bereinigen
    deleteFolder("sdmc:/switch/.packages/cleanup Sigpatches");
    deleteFolder("sdmc:/atmosphere/exefs_patches/es_patches");
    deleteFolder("sdmc:/atmosphere/exefs_patches/nfim_ctest");
    deleteFolder("sdmc:/atmosphere/kip_patches/fs_patches");
    deleteFolder("sdmc:/atmosphere/kip_patches/loader_patches");
    deleteFolder("sdmc:/atmosphere/kip_patches");
    deleteFile("sdmc:/bootloader/patches.ini");

    // Sich selbst bereinigen
    deleteFolder("sdmc:/atmosphere/contents/010000000000DA7A/flags");
    deleteFolder("sdmc:/atmosphere/contents/010000000000DA7A");

    return 0;
}
