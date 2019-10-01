#ifndef COMMANDBUCKET_INL_
#define COMMANDBUCKET_INL_

#include "cmd/commandpacket.inl"

#include <algorithm>
#include <cstring>

namespace cmd
{
template <typename Key>
CommandBucket<Key>::CommandBucket(size_t num_packets)
: m_sorted_indices{new size_t[num_packets]},
  m_keys{new Key[num_packets]},
  m_packets{new CommandPacket[num_packets]},
  m_capacity{num_packets}
{}

template <typename Key>
CommandBucket<Key>::CommandBucket(CommandBucket<Key> && other)
{
    swap(*this, other);
}

template <typename Key>
CommandBucket<Key>::~CommandBucket()
{
	Clear();

    delete[] m_sorted_indices;
    delete[] m_keys;
    delete[] m_packets;
}

template <typename Key>
CommandBucket<Key> & CommandBucket<Key>::operator=(CommandBucket<Key> other)
{
    swap(*this, other);
}

template <typename Key>
void CommandBucket<Key>::Submit()
{
    // ... same as before
    for (unsigned int i = 0; i < m_size; ++i)
    {
        // ... same as before
        CommandPacket packet     = m_packets[i];
        CommandPacket old_packet = nullptr;
        do
        {
            SubmitPacket(packet);
            old_packet = packet;
            packet     = commandPacket::LoadNextCommandPacket(old_packet);
            //::operator delete(old_packet);

        } while (packet != nullptr);
    }
}

template <typename Key>
void CommandBucket<Key>::Clear()
{
    for (unsigned int i = 0; i < m_size; ++i)
    {
        // ... same as before
        CommandPacket packet     = m_packets[i];
        CommandPacket old_packet = nullptr;
        do
        {
            old_packet = packet;
            packet     = commandPacket::LoadNextCommandPacket(old_packet);
            ::operator delete(old_packet);

        } while (packet != nullptr);
    }

    m_size = 0;
}

template <typename Key>
void CommandBucket<Key>::SubmitPacket(const CommandPacket packet)
{
    const BackendDispatchFunction function = commandPacket::LoadBackendDispatchFunction(packet);
    const void *                  command  = commandPacket::LoadCommand(packet);
    function(command);
}

template <typename Key>
template <typename U>
U * CommandBucket<Key>::AddCommand(Key key, size_t auxMemorySize)
{
    assert(m_size < m_capacity);

    if (m_size >= m_capacity)
    {
        return nullptr;
    }

    CommandPacket packet = commandPacket::Create<U>(auxMemorySize);

    // store key and pointer to the data
    {
        // TODO: add some kind of lock or atomic operation here
        const unsigned int current = m_size++;
        m_sorted_indices[current]  = current;
        m_keys[current]            = key;
        m_packets[current]         = packet;
    }

    /*

    {
        int32_t id = tlsThreadId;

        // fetch number of remaining packets in this layer for the thread with this id
        int32_t remaining = m_tlsRemaining[id];
        int32_t offset = m_tlsOffset[id];

        if (remaining == 0)
        {
            // no more storage in this block remaining, get new one
            offset = core::atomic::Add(&m_current, 32);
            remaining = 32;

            // write back
            m_tlsOffset[id] = offset;
        }

        int32_t current = offset + (32-remaining);
        m_keys[current] = key;
        m_packets[current] = packet;
        --remaining;

        // write back
        m_tlsRemaining[id] = remaining;
    }
  */

    commandPacket::StoreNextCommandPacket(packet, nullptr);
    commandPacket::StoreBackendDispatchFunction(packet, U::DISPATCH_FUNCTION);

    return commandPacket::GetCommand<U>(packet);
}

template <typename Key>
size_t CommandBucket<Key>::size()
{
    return m_size;
}

template <typename Key>
size_t CommandBucket<Key>::capacity()
{
    return m_capacity;
}

template <typename Key>
void CommandBucket<Key>::Merge(size_t * scratch_list,
                               size_t   index_left,
                               size_t   index_right,
                               size_t   end_index)
{
    memcpy(&scratch_list[index_left],
           &m_sorted_indices[index_left],
           sizeof(size_t) * (end_index - index_left + 1));

    auto sorted_index = index_left;

    auto end_left  = index_right - 1;
    auto end_right = end_index;

    while (index_left <= end_left || index_right <= end_right)
    {
        // check if either list is finished

        if (index_right > end_right)
        {
            // right side is finished
            m_sorted_indices[sorted_index++] = scratch_list[index_left++];
        }
        else if (index_left > end_left)
        {
            // left side is finished
            m_sorted_indices[sorted_index++] = scratch_list[index_right++];
        }
        else if (m_keys[scratch_list[index_left]] > m_keys[scratch_list[index_right]])
        {
            // right side is smaller
            m_sorted_indices[sorted_index++] = scratch_list[index_right++];
        }
        else
        {
            // left side is smaller
            m_sorted_indices[sorted_index++] = scratch_list[index_left++];
        }
    }
}

template <typename Key>
void CommandBucket<Key>::MergeSort(size_t * scratch_list, size_t start_index, size_t end_index)
{
    assert(start_index <= end_index);

    auto difference = end_index - start_index;

    if (difference == 0)
    {
        return;
    }
    else if (difference > 1)
    {
        // split and mergesort both sides
        auto middle_index = start_index + difference / 2;

        MergeSort(scratch_list, start_index, middle_index);
        MergeSort(scratch_list, middle_index + 1, end_index);

        // merge here

        Merge(scratch_list, start_index, middle_index + 1, end_index);
    }
    else
    {
        auto key_index_left  = m_sorted_indices[start_index];
        auto key_index_right = m_sorted_indices[end_index];

        if (m_keys[key_index_left] > m_keys[key_index_right])
        {
            using std::swap;

            swap(m_sorted_indices[start_index], m_sorted_indices[end_index]);
        }
    }
}

template <typename Key>
void CommandBucket<Key>::Sort()
{
    if (m_size <= 1)
        return;

    size_t * scratch_list = new size_t[m_size];

    MergeSort(scratch_list, 0, m_size - 1);

    delete[] scratch_list;
}

template <typename T>
void swap(CommandBucket<T> & first, CommandBucket<T> & second)
{
    using std::swap;

    swap(first.m_sorted_indices, second.m_sorted_indices);
    swap(first.m_keys, second.m_keys);
    swap(first.m_packets, second.m_packets);
    swap(first.m_capacity, second.m_capacity);
    swap(first.m_size, second.m_size);
}

}; // namespace cmd

#endif