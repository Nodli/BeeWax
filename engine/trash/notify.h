#if 0
//utest::inotify_example();

// NOTE(hugo): use example.txt file as target
void inotify_example(){
    s32 inotify_descriptor = inotify_init();
    if(inotify_descriptor == -1){
        LOG_ERROR("Failed inotify_init() %s", strerror(errno));
    }

    const char* const path = "./data/example.txt";

    s32 watch_descriptor = inotify_add_watch(inotify_descriptor, path, IN_ALL_EVENTS);
    if(watch_descriptor == -1){
        LOG_ERROR("Failed inotify_add_watch(%s) %s", path, strerror(errno));
    }

    constexpr u32 inotify_nevent = 16;
    constexpr u32 inotify_buffer_size = inotify_nevent * (sizeof(inotify_event) + NAME_MAX + 1u);
    char inotify_buffer[inotify_buffer_size];

    LOG_TRACE("inotify_buffer_size %d", inotify_buffer_size);

    while(true){
        LOG_TRACE("Reading...");

        s32 bytes = read(inotify_descriptor, inotify_buffer, inotify_buffer_size);
        LOG_TRACE("Received bytes = %d", bytes);

        if(bytes == -1){
            LOG_TRACE("Error during read()");
            break;
        }

        s32 offset = 0;
        while(offset < bytes){
            inotify_event* event = (inotify_event*)(inotify_buffer + offset);
            LOG_inotify_event(event);
            offset = offset + sizeof(inotify_event) + event->len;
            LOG_TRACE("Current offset = %d", offset);
        }
        LOG_TRACE("");
    }

    s32 removal_status = inotify_rm_watch(inotify_descriptor, watch_descriptor);
    if(removal_status == -1){
        LOG_ERROR("Failed inotify_rm_watch(%s) %s\n", path, strerror(errno));
    }

    close(inotify_descriptor);
}
#endif

#if 0
// NOTE(hugo): Those are linux stuff that should be put in a platform layer file
#include <errno.h>          // strerror
#include <sys/epoll.h>      // epoll_
#include <sys/inotify.h>    // inotify_
#include <unistd.h>         // read, close

struct File_Entry{
    s32 watch_descriptor = -1;
};
struct File_System{
    void initialize(u32 inumber_of_files);
    void terminate();

    u32 open_file(const char* const path);
    void close_file(u32 file_handle);

    s32 inotify_descriptor = -1;
    File_Entry* array = nullptr;
    u32 nfiles = 0;
};

void run_test_file_watcher(){

    s32 inotify_descriptor = inotify_init();
    if(inotify_descriptor == -1){
        LOG_ERROR("Failed inotify_init() %s", strerror(errno));
    }

    s32 epoll_descriptor = epoll_create(1); // NOTE(hugo): argument is dropped but must be > 0
    if(epoll_descriptor == -1){
        LOG_ERROR("Failed epoll_create %s", strerror(errno));
    }

    epoll_event event;
    event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    s32 epoll_register_status = epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, inotify_descriptor, &event);
    if(epoll_register_status == -1){
        LOG_ERROR("Failed epoll_ctl %s", strerror(errno));
    }

    //const char* const path = "./data/example.txt";
    const char* const path = "./data";

    //s32 watch_descriptor = inotify_add_watch(inotify_descriptor, path, IN_MODIFY | IN_ATTRIB | IN_MOVE);
    s32 watch_descriptor = inotify_add_watch(inotify_descriptor, path, IN_ALL_EVENTS);
    if(watch_descriptor == -1){
        LOG_ERROR("Failed inotify_add_watch(%s) %s", path, strerror(errno));
    }

    constexpr u32 inotify_nevent = 16;
    constexpr u32 inotify_buffer_size = inotify_nevent * (sizeof(inotify_event) + NAME_MAX + 1u);
    char inotify_buffer[inotify_buffer_size];

    LOG_TRACE("inotify_buffer_size %d", inotify_buffer_size);

    constexpr s32 epoll_nevent = 16;
    epoll_event* epoll_event_array = (epoll_event*)malloc(epoll_nevent * sizeof(epoll_event));
    s32 epoll_wait_return = epoll_wait(epoll_descriptor, epoll_event_array, epoll_nevent, 0);
    if(epoll_wait_return == -1){
        LOG_ERROR("Failed epoll_wait %s", strerror(errno));
    }else{
        assert(epoll_wait_return >= 0);
        for(u32 ievent = 0; ievent != (u32)epoll_nevent; ++ievent){
        }
    }

    while(true){
        LOG_TRACE("Reading...");

        s32 bytes = read(inotify_descriptor, inotify_buffer, inotify_buffer_size);
        LOG_TRACE("Received bytes = %d", bytes);

        if(bytes == -1){
            LOG_TRACE("Error during read()");
            break;
        }

        s32 offset = 0;
        while(offset < bytes){
            inotify_event* event = (inotify_event*)(inotify_buffer + offset);
            LOG_inotify_event(event);
            offset = offset + sizeof(inotify_event) + event->len;
            LOG_TRACE("Current offset = %d", offset);

            // NOTE(hugo): IN_DELETE_SELF -> remove and readd ?
            //if(event->mask & IN_IGNORED)
        }
        LOG_TRACE("");
    }

    s32 removal_status = inotify_rm_watch(inotify_descriptor, watch_descriptor);
    if(removal_status == -1){
        LOG_ERROR("Failed inotify_rm_watch(%s) %s\n", path, strerror(errno));
    }

    close(inotify_descriptor);
}
#endif

#if 0
void File_System::initialize(u32 inumber_of_files){
    s32 inotify_descriptor = inotify_init();
    if(inotify_descriptor == -1){
        LOG_ERROR("Failed inotify_init() %s\n", strerror(errno));
    }

    assert(!files);
    files = (File_Entry*)malloc(sizeof(File_Entry) * inumber_of_files);
    if(files){
    }
    nfiles = inumber_of_files;
}

void File_System::terminate(){
}
#endif
