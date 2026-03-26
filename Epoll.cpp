#include"Epoll.h"

Epoll::Epoll()                                // 在构造函数中创建了epollfd_。
{
     if((epollfd_ = epoll_create(1)) == -1)//创建epoll句柄
     {
        printf("epoll_create(1) failed(%d).\n",errno);exit(-1);
     }
}

Epoll::~Epoll()                              // 在析构函数中关闭epollfd_。
{
    close(epollfd_);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Epoll::updatechannel(Channel *ch)
{
    epoll_event ev;        // 声明事件的数据结构。
    ev.data.ptr=ch;        // 指定channel。
    ev.events=ch->events(); // 指定事件。

    if (ch->inpoll())      // 如果channel已经在树上了。
    {
        if (epoll_ctl(epollfd_,EPOLL_CTL_MOD,ch->fd(),&ev)==-1)
        {
            perror("epoll_ctl() failed.\n"); exit(-1);
        }
    }
    else                   // 如果channel不在树上。
    {
        if (epoll_ctl(epollfd_,EPOLL_CTL_ADD,ch->fd(),&ev)==-1)
        {
            perror("epoll_ctl() failed.\n"); exit(-1);
        }
        ch->setinepoll();  // 把channel的inepoll_成员设置为true。
    }
}

void Epoll::removechannel(Channel *ch)                         //从红黑树上删除Channel。
{
    if (ch->inpoll())      // 如果channel已经在树上了。
    {
        printf("removechannel\n");
      if (epoll_ctl(epollfd_,EPOLL_CTL_DEL,ch->fd(),0)==-1)
      {
        perror("epoll_ctl() failed.\n"); exit(-1);
      }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<Channel *> Epoll::loop(int timeout)  // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回
{
    std::vector<Channel *>channels;
    bzero(events_,sizeof(events_));

    int nums = epoll_wait(epollfd_,events_,MaxEvents,timeout);//监督有无事件发生
    if(nums < 0)
    {
     perror("epoll_wait");
     exit(-1);
    }
    if(nums == 0)
    {
     //如果epoll_wait()超时,表示系统很空闲,返回的channels将为空。
     //printf("epoll_wait timeout.\n"); 
     return channels;
    }
    for(int ii = 0;ii<nums;ii++)
    {
      
     Channel *ch=(Channel *)events_[ii].data.ptr;    // 取出已发生事件的channel。
     ch->setrevents(events_[ii].events);            // 设置channel的revents_成员。
     channels.push_back(ch);
    }
    return channels;
        
}