// ============================================================================
// Virtual File System (VFS)
// Абстрактный слой для файловых систем
// ============================================================================

#include "../include/vfs.h"
#include "../include/heap.h"

// Внешние функции
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);

// Root filesystem node
vfs_node_t* vfs_root = NULL;

// ============================================================================
// Функция: vfs_init
// Инициализирует VFS
// ============================================================================
void vfs_init(void) {
    terminal_write_line("[VFS] Initializing Virtual File System...");

    // Root будет установлен при монтировании файловой системы
    vfs_root = NULL;

    terminal_write_line("[OK] VFS initialized");
}

// ============================================================================
// Функция: vfs_read
// Читает из VFS node
// ============================================================================
uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (!node) {
        return 0;
    }

    // Если есть обработчик чтения
    if (node->read) {
        return node->read(node, offset, size, buffer);
    }

    return 0;
}

// ============================================================================
// Функция: vfs_write
// Записывает в VFS node
// ============================================================================
uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (!node) {
        return 0;
    }

    // Если есть обработчик записи
    if (node->write) {
        return node->write(node, offset, size, buffer);
    }

    return 0;
}

// ============================================================================
// Функция: vfs_open
// Открывает VFS node
// ============================================================================
void vfs_open(vfs_node_t* node, uint32_t flags) {
    if (!node) {
        return;
    }

    (void)flags;  // Unused for now

    // Если есть обработчик открытия
    if (node->open) {
        node->open(node);
    }
}

// ============================================================================
// Функция: vfs_close
// Закрывает VFS node
// ============================================================================
void vfs_close(vfs_node_t* node) {
    if (!node) {
        return;
    }

    // Если есть обработчик закрытия
    if (node->close) {
        node->close(node);
    }
}

// ============================================================================
// Функция: vfs_readdir
// Читает запись директории
// ============================================================================
dirent_t* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if (!node) {
        return NULL;
    }

    // Проверить, что это директория
    if ((node->flags & 0x7) != VFS_DIRECTORY) {
        return NULL;
    }

    // Если есть обработчик readdir
    if (node->readdir) {
        return node->readdir(node, index);
    }

    return NULL;
}

// ============================================================================
// Функция: vfs_finddir
// Находит файл в директории
// ============================================================================
vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name) {
    if (!node) {
        return NULL;
    }

    // Проверить, что это директория
    if ((node->flags & 0x7) != VFS_DIRECTORY) {
        return NULL;
    }

    // Если есть обработчик finddir
    if (node->finddir) {
        return node->finddir(node, name);
    }

    return NULL;
}
