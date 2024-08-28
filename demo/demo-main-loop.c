// Copyright © 2024 <dingjing@live.cn>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 24-8-27.
//

#include <c/clib.h>
#include <glib.h>
#include <gio/gio.h>


static void do_action(char* cmd);
static void print_standard_info (GFile* file);
static void handle_mount_changed(GVolumeMonitor* monitor, GMount* mount, gpointer data);
static void handle_volume_changed(GVolumeMonitor* monitor, GVolume* volume, gpointer data);



int main (int argc, char* argv[])
{
    CMainLoop* loop = c_main_loop_new(NULL, true);

    GVolumeMonitor* monitor = g_volume_monitor_get();

    g_signal_connect(G_OBJECT(monitor), "volume-added", G_CALLBACK(handle_volume_changed), "volume-added");
    g_signal_connect(G_OBJECT(monitor), "volume-removed", G_CALLBACK(handle_volume_changed), "volume-removed");

    g_signal_connect(G_OBJECT(monitor), "mount-added", G_CALLBACK(handle_mount_changed), "mount-added");
    g_signal_connect(G_OBJECT(monitor), "mount-removed", G_CALLBACK(handle_mount_changed), "mount-removed");

    c_main_loop_run(loop);

    return 0;
}

static void do_action(char* cmd)
{
    g_print("[do_action] start to exec: %s\n", cmd);
    GError* error = NULL;
    g_spawn_command_line_sync(cmd, NULL, NULL, NULL, &error);
    if (error) {
        g_print("[do_action] exec '%s' failed: %s\n", cmd, error->message);
        g_error_free(error);
    }
    g_print("[do_action] exec done\n");
}

static void handle_volume_changed(GVolumeMonitor* monitor, GVolume* volume, gpointer data)
{
    char* ev = (char*)data;
    g_print("[handle_volume_changed] event: %s\n", ev);
    if (0 == g_strcmp0(ev, "volume-removed")) {
        return;
    }

    g_autofree char* path = g_volume_get_identifier(volume, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
    if (path) g_print("unix id: %s\n", path);
    g_autofree char* cmd = g_strdup_printf("gvfs-mount -d %s", path);

    do_action(cmd);
}

static void print_standard_info (GFile* file)
{
    g_autoptr(GFileInfo) fileInfo = g_file_query_info (file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, NULL);

    printf ("target-uri: %s\n", g_file_info_get_attribute_string(fileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI));
}

static void handle_mount_changed(GVolumeMonitor* monitor, GMount* mount, gpointer data)
{
    char* ev = (char*)data;
    g_print("[handle_mount_changed] event: %s\n", ev);

    g_autoptr(GFile) file = g_mount_get_default_location(mount);
    g_autofree char* path = g_file_get_path(file);

    g_autoptr(GFile) file1 = g_file_new_for_path (path);
    g_autofree char* uri = g_file_get_uri(file1);
    g_autofree char* schema = g_file_get_uri_scheme(file1);

    printf ("path: %s\n", path);
    printf ("uri: %s\n", uri);
    printf ("schema: %s\n", schema);

    //do_action(cmd);
}

