#pragma once

template<unsigned capacity_, typename Type=uint8_t>
class RingBuffer
{
    Type buffer_[capacity_];
    unsigned start_=0, size_=0;
public:
    bool push_back(const Type x)
    {
        if(size_>=capacity_) return false;
        if(start_+size_<capacity_)
            buffer_[start_+size_]=x;
        else
            buffer_[start_+size_-capacity_]=x;
        ++size_;
        return true;
    }
    void clear()
    {
        size_=0;
    }

    Type& operator[](unsigned i)
    {
        if(start_+i<capacity_) return buffer_[start_+i];
        return buffer_[start_+i-capacity_];
    }
    Type operator[](unsigned i) const
    {
        return const_cast<RingBuffer*>(this)[i];
    }

    Type& back()       { return (*this)[size_-1]; }
    Type  back() const { return (*this)[size_-1]; }

    Type& front()       { return (*this)[0]; }
    Type  front() const { return (*this)[0]; }

    Type pop_front()
    {
        if(empty()) return {};

        const auto value=front();
        ++start_;
        --size_;
        if(start_ >= capacity_)
            start_ -= capacity_;
        return value;
    }

    unsigned size() const { return size_; }
    bool empty() const { return size_==0; }
    constexpr unsigned capacity() const { return capacity_; }
};
