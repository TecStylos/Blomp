#pragma once

#include <queue>
#include <memory>

namespace Blomp
{
    template <class T>
    class BlockPool
    {
    public:
        template <typename ...Args>
        std::shared_ptr<T> take(Args...);
        void give(std::shared_ptr<T> obj);
    private:
        std::queue<std::shared_ptr<T>> m_queue;
    };

    template <class T>
    template <typename ...Args>
    std::shared_ptr<T> BlockPool<T>::take(Args... args)
    {
        if (m_queue.empty())
            return std::shared_ptr<T>(new T(args...));

        auto temp = m_queue.front();
        m_queue.pop();
        *temp = T(args...);
        return temp;
    }

    template <class T>
    void BlockPool<T>::give(std::shared_ptr<T> obj)
    {
        m_queue.push(obj);
    }

    extern BlockPool<class ColorBlock> ColorBlockPool;
}