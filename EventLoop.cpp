#include"EventLoop.h"

int createtimerfd(int sec=30)
{
  int tfd=timerfd_create(CLOCK_MONOTONIC,TFD_CLOEXEC|TFD_NONBLOCK); // 创建timerfd。
  struct itimerspec timeout;          // 定时时间的数据结构。
  memset(&timeout,0,sizeof(struct itimerspec));
  timeout.it_value.tv_sec = sec;       // 定时时间为5秒。
  timeout.it_value.tv_nsec = 0;
  timerfd_settime(tfd,0,&timeout,0); // 开始计时。alarm(5)
  return tfd;
}
// 在构造函数中创建Epoll对象ep_。
EventLoop::EventLoop(bool mainloop,int timetvl,int timeout):mainloop_(mainloop),ep_(new Epoll),
          timetvl_(timetvl),timeout_(timeout),stop_(false),
          wakeupfd_(eventfd(0,EFD_NONBLOCK )),wakechannel_(new Channel(this,wakeupfd_)),
          timerfd_(createtimerfd(timeout_)), timerchannel_(new Channel(this,timerfd_))            
{
  wakechannel_->setreadcallback(std::bind(&EventLoop::handlewakeup,this));
  wakechannel_->enablereading();

  timerchannel_->setreadcallback(std::bind(&EventLoop::handletimer,this));
  timerchannel_->enablereading();
}

EventLoop::~EventLoop()                                  // 在析构函数中销毁ep_。
{
   // delete ep_;
}
//////////////////////////////////////////////////////////////////////////////////////////////
void EventLoop::run()                                    // 运行事件循环。
{
  //printf("EventLoop::run() thread is %d.\n",syscall(SYS_gettid));
  threadid_=syscall(SYS_gettid);     //获取事件循环所在线程的id。
  while(stop_==false)
  {
     
    std::vector<Channel *>channels=ep_->loop(10*1000);   //等待监视fd有事件发生
    //如果channels为空,表示超时,回调TcpServer::epolltimeout()。

    if (channels.size ()==0)
    {
     epolltimeoutcallback_(this);                        //回调TcpServer类的epooltimwout()
    }
    else 
    {
      for(auto &ch:channels)
     {   
      ch->handleevent();
     }
    }
  } 
   
}

void EventLoop::stop()                                                      //停止事件循环。
{
  stop_=true;
  wakeup();             //唤醒事件循环,如果没有这行代码,事件循环将在下次闹钟响时侯epoll wait()超时时才会停下来。
}


void EventLoop::updatechannel(Channel *ch)               //把channel添加/更新到红黑树上,channel中有fd,也有需要监视的事件
{
  ep_->updatechannel(ch);
}

void EventLoop::removechannel(Channel *ch)                                  //从红黑树上删除channel。
{
  ep_->removechannel(ch);

}

void EventLoop::setepolltimeoutcallback(std::function<void(EventLoop*)> fn)//设置epoll wait()超时的回调函数
{
  epolltimeoutcallback_=fn;
}

bool EventLoop::isinloopthread()           //判断线程是否为事件循环线程
{
  return   threadid_ == syscall(SYS_gettid);     
}

void EventLoop::queueinloop(std::function<void()> fn)  //    把任务添加到队列中。
{
  {
    std::lock_guard<std::mutex> gd(mutex_);    // 给任务队列加锁。
    taskqueue_.push(fn);                       // 任务入队。
  }
  wakeup();                         //唤醒事件循环
}

void EventLoop::wakeup()                                //用eventfd唤醒事件循环线程。
{
  uint64_t val=1;
  write(wakeupfd_,&val,sizeof(val));
}

void EventLoop::handlewakeup()                        //事件循环线程被eventfd唤醒后执行的函数。
{
  //printf("handlewakeup() thread id is %d.\n",syscall(SYS_gettid));

  uint64_t val;
  read(wakeupfd_,&val,sizeof(val));  // 从eventfd中读取出数据，如果不读取，eventfd的读事件会一直触发。

  std::function<void()> fn;
  std::lock_guard<std::mutex> gd(mutex_);  // 给任务队列加锁。
  // 执行队列中全部的发送任务。
  while (taskqueue_.size()>0)
  {
    fn=std::move(taskqueue_.front());  // 出队一个元素。
    taskqueue_.pop();
    fn();                              // 执行任务。
  }
}


void EventLoop::handletimer()                         //闹钟响时执行的函数。
{
  //重新记时
  struct itimerspec timeout;          // 定时时间的数据结构。
  memset(&timeout,0,sizeof(struct itimerspec));
  timeout.it_value.tv_sec = timetvl_;       // 定时时间为5秒。
  timeout.it_value.tv_nsec = 0;
  timerfd_settime(timerfd_,0,&timeout,0); // 开始计时。alarm(5)

  if (mainloop_)
  {
    //printf("主事件循环的闹钟时间到了。\n");

  }
  else
  {
    //printf("从事件循环的闹钟时间到了。\n");
    time_t now=time(0);    // 获取当前时间。

    for (auto it=conns_.begin();it!=conns_.end();)
    {
     if (it->second->timeout(now,timeout_))
     {
      timercallback_(it->first);     // 从TcpServer的map中删除超时的conn。
      std::lock_guard<std::mutex>gd(mmutex_);
      it=conns_.erase(it);            // 从EventLoop的map中删除超时的conn。
     }else it++;
    }

  }
}

void EventLoop::newconnection(spConnection conn)      //把Connection对象保存在conns_中。
{
  std::lock_guard<std::mutex>gd(mmutex_);
  conns_[conn->fd()]=conn;
}
void EventLoop::settimercallback(std::function<void(int)> fn)    //将被设置为TcpServer::removSanh
{
  timercallback_=fn; 
}
