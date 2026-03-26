#include"Buffer.h"

Buffer::Buffer(uint16_t sep):sep_(sep)
{

}
Buffer::~Buffer()
{

}

void Buffer::append(const char *data,size_t size)  // 把数据追加到buf_中。
{
    buf_.append(data,size);
}

void Buffer::appendwithsep(const char *data,size_t size)  // 把数据追加到buf_中,附加报文头部
{
    if (sep_==0)        // 没有分隔符。
    {
      buf_.append(data,size);        // 处理报文内容。
    }
    else if (sep_==1)   // 四字节的报头。
    {
      buf_.append((char*)&size,4);   // 处理报文长度（头部）。
      buf_.append(data,size);        // 处理报文内容。
    }
    else if (sep_==2)   // HTTP协议。
    {
      buf_.append(data,size);        // HTTP响应已经自带头部，直接追加。
    }

}

void Buffer::erase(size_t pos,size_t nn)        // 从buf_的pos开始，删除nn个字节，pos从0开始。
{
    buf_.erase(pos,nn);
}

size_t Buffer::size()                              // 返回buf_的大小。
{
    return buf_.size();
}
const char *Buffer::data()                         // 返回buf_的首地址。
{
    return buf_.data();
}
void Buffer::clear()                               // 清空buf_。
{
    return buf_.clear();
}

// 从buf_中拆分出一个报文，存放在ss中，如果buf_中没有报文，返回false。
bool Buffer::pickmessage(std::string &ss)
{
	if (buf_.size()==0) return false;

	if (sep_==0)        // 没有分隔符。
	{
		ss=buf_;
		buf_.clear();
	}
	else if (sep_==1)   // 四字节的报头。
	{
		int len;
		memcpy(&len,buf_.data(),4);  // 从buf_中获取报文头部。
		// 如果buf_中的数据量小于报文头部，说明buf_中的报文内容不完整。
		if (buf_.size()<len+4) return false;

		ss = buf_.substr(4,len);  // 从buf_中获取一个报文。
		buf_.erase(0,len+4);                    // 从buf_中删除刚才已获取的报文。
	}
	else if (sep_==2)   // HTTP协议的分隔符 \r\n\r\n
	{
		size_t pos = buf_.find("\r\n\r\n");
		if (pos == std::string::npos) return false;

		ss = buf_.substr(0, pos + 4);  // 获取完整的HTTP头部（可能包含body的一小部分，为了简化，当前假设只处理头部或简单的GET）
		buf_.erase(0, pos + 4);        // 从buf_中删除
	}

	return true;
}
/*      GDB跟踪调试
int main()
{
    std::string s1="aaaaaaaaaaaaaaab";
    Buffer buf(1);
    buf.appendwithsep(s1.data(),s1.size());
    Buffer buf ;
    buf.pickmessage(s2);
    printf("s2=%s\n",s2.c_str());
}
*/