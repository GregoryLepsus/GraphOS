// ============================================================================
// InitRD File System
// Простая RAM-based файловая система для загрузки начальных файлов
// ============================================================================

#include "../include/initrd.h"
#include "../include/vfs.h"
#include "../include/heap.h"

// Внешние функции
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void print_dec(uint32_t value);

// InitRD данные
static initrd_header_t* initrd_header;
static initrd_file_header_t* file_headers;
static vfs_node_t* initrd_root;
static vfs_node_t* initrd_files;
static uint32_t initrd_location;

// ============================================================================
// Функция: initrd_read
// Читает из initrd файла
// ============================================================================
static uint32_t initrd_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    initrd_file_header_t* header = &file_headers[node->inode];

    // Проверить границы
    if (offset >= header->length) {
        return 0;
    }

    // Ограничить размер
    if (offset + size > header->length) {
        size = header->length - offset;
    }

    // Копировать данные
    uint8_t* src = (uint8_t*)(initrd_location + header->offset + offset);
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = src[i];
    }

    return size;
}

// ============================================================================
// Функция: initrd_readdir
// Читает директорию
// ============================================================================
static dirent_t* initrd_readdir(vfs_node_t* node, uint32_t index) {
    (void)node;  // Root directory

    if (index >= initrd_header->file_count) {
        return NULL;
    }

    // Создать dirent
    dirent_t* dirent = (dirent_t*)kmalloc(sizeof(dirent_t));
    if (!dirent) {
        return NULL;
    }

    // Копировать имя файла
    initrd_file_header_t* header = &file_headers[index];
    for (int i = 0; i < 64; i++) {
        dirent->name[i] = header->name[i];
    }
    dirent->inode = index;

    return dirent;
}

// ============================================================================
// Функция: initrd_finddir
// Находит файл в директории
// ============================================================================
static vfs_node_t* initrd_finddir(vfs_node_t* node, const char* name) {
    (void)node;  // Root directory

    // Искать файл по имени
    for (uint32_t i = 0; i < initrd_header->file_count; i++) {
        initrd_file_header_t* header = &file_headers[i];

        // Сравнить имена
        bool match = true;
        for (int j = 0; j < 64; j++) {
            if (header->name[j] != name[j]) {
                match = false;
                break;
            }
            if (name[j] == '\0') {
                break;
            }
        }

        if (match) {
            return &initrd_files[i];
        }
    }

    return NULL;
}

// ============================================================================
// Функция: initrd_init
// Инициализирует initrd файловую систему
// ============================================================================
vfs_node_t* initrd_init(uint32_t location) {
    terminal_write_line("[INITRD] Initializing InitRD filesystem...");

    initrd_location = location;
    initrd_header = (initrd_header_t*)location;

    // Проверить magic number
    if (initrd_header->magic != 0xBF) {
        terminal_write_line("[INITRD] Invalid magic number!");
        return NULL;
    }

    terminal_write("  Files: ");
    print_dec(initrd_header->file_count);
    terminal_write_line("");

    // Получить file headers
    file_headers = (initrd_file_header_t*)(location + sizeof(initrd_header_t));

    // Создать VFS nodes для файлов
    initrd_files = (vfs_node_t*)kmalloc(sizeof(vfs_node_t) * initrd_header->file_count);
    if (!initrd_files) {
        terminal_write_line("[INITRD] ERROR: Cannot allocate file nodes!");
        return NULL;
    }

    // Инициализировать файлы
    for (uint32_t i = 0; i < initrd_header->file_count; i++) {
        initrd_file_header_t* header = &file_headers[i];
        vfs_node_t* file = &initrd_files[i];

        // Копировать имя
        for (int j = 0; j < 64; j++) {
            file->name[j] = header->name[j];
        }

        file->mask = 0;
        file->uid = 0;
        file->gid = 0;
        file->flags = VFS_FILE;
        file->inode = i;
        file->length = header->length;
        file->impl = 0;

        // Установить обработчики
        file->read = initrd_read;
        file->write = NULL;
        file->open = NULL;
        file->close = NULL;
        file->readdir = NULL;
        file->finddir = NULL;
        file->ptr = NULL;
    }

    // Создать root directory node
    initrd_root = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!initrd_root) {
        kfree(initrd_files);
        terminal_write_line("[INITRD] ERROR: Cannot allocate root node!");
        return NULL;
    }

    // Инициализировать root
    initrd_root->name[0] = '/';
    initrd_root->name[1] = '\0';
    initrd_root->mask = 0;
    initrd_root->uid = 0;
    initrd_root->gid = 0;
    initrd_root->flags = VFS_DIRECTORY;
    initrd_root->inode = 0;
    initrd_root->length = 0;
    initrd_root->impl = 0;

    // Установить обработчики
    initrd_root->read = NULL;
    initrd_root->write = NULL;
    initrd_root->open = NULL;
    initrd_root->close = NULL;
    initrd_root->readdir = initrd_readdir;
    initrd_root->finddir = initrd_finddir;
    initrd_root->ptr = NULL;

    terminal_write_line("[OK] InitRD filesystem initialized");
    return initrd_root;
}
