//
// Created by dingjing on 24-3-7.
//
#include "../glib/glog.h"

int main (C_UNUSED int argc, C_UNUSED char* argv[])
{
    C_GLOG_VERB("%s:%d", "ss", 1);
    C_GLOG_DEBUG("%s:%d", "ss", 1);
    C_GLOG_INFO("%s:%d", "ss", 1);
    C_GLOG_WARNING("%s:%d", "ss", 1);
    C_GLOG_CRIT("%s:%d", "ss", 1);

    g_warning("wwwwwwwwwwwwwwwwwww");

    return 0;
}
