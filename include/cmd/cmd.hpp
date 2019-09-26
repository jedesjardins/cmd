#ifndef JED_CMD_HPP_
#define JED_CMD_HPP_

#include <stddef.h>

namespace cmd
{

// Backend Dispatch Definition

using BackendDispatchFunction = void (*)(void const*);

// Command Packet Stuff

using CommandPacket = void*;

namespace commandPacket
{

	static const size_t OFFSET_NEXT_COMMAND_PACKET = 0u;
	static const size_t OFFSET_BACKEND_DISPATCH_FUNCTION = OFFSET_NEXT_COMMAND_PACKET + sizeof(CommandPacket);
	static const size_t OFFSET_COMMAND = OFFSET_BACKEND_DISPATCH_FUNCTION + sizeof(BackendDispatchFunction);

	template <typename T>
	size_t GetSize(size_t auxMemorySize);

	template <typename T>
	CommandPacket Create(size_t auxMemorySize);

	CommandPacket* GetNextCommandPacket(CommandPacket packet);

	template <typename T>
	CommandPacket* GetNextCommandPacket(T* command);

	BackendDispatchFunction* GetBackendDispatchFunction(CommandPacket packet);

	template <typename T>
	T* GetCommand(CommandPacket packet);

	template <typename T>
	char* GetAuxiliaryMemory(T* command);

	void StoreNextCommandPacket(CommandPacket packet, CommandPacket nextPacket);

	template <typename T>
	void StoreNextCommandPacket(T* command, CommandPacket nextPacket);
	 
	void StoreBackendDispatchFunction(CommandPacket packet, BackendDispatchFunction dispatchFunction);

	CommandPacket const LoadNextCommandPacket(CommandPacket const packet);

	BackendDispatchFunction const LoadBackendDispatchFunction(CommandPacket const packet);

	void const* LoadCommand(CommandPacket const packet);

	template <typename U, typename V>
	U* AppendCommand(V* command, size_t auxMemorySize);

}; // namespace commandPacket

// Command Bucket Stuff

template <typename Key>
class CommandBucket
{
public:

	CommandBucket(size_t num_packets);
	CommandBucket(CommandBucket<Key> && other);
	~CommandBucket();

	CommandBucket& operator=(CommandBucket<Key> other);

	void Submit();
	void SubmitPacket(const CommandPacket packet);
	void Clear();

	void Merge(size_t * scratch_list, size_t first_index, size_t second_index, size_t end_index);
	void MergeSort(size_t * scratch_list, size_t start_index, size_t end_index);
	void Sort();

	template <typename U>
	U* AddCommand(Key key, size_t auxMemorySize);

	template <typename U, typename V>
	U* AppendCommand(V* last_command, size_t auxMemorySize);

private:

 	template <typename T>
	friend void swap(CommandBucket<T> & first, CommandBucket<T> & second);

	size_t * m_sorted_indices{nullptr};
	Key * m_keys{nullptr};
	CommandPacket * m_packets{nullptr};
	size_t m_capacity{0};
	size_t m_size{0};
};

template <typename T>
void swap(CommandBucket<T> & first, CommandBucket<T> & second);

}; // namespace cmd

#ifndef JED_CMD_EXCLUDE_INL
#include "commandpacket.inl"
#include "commandbucket.inl"
#endif




#endif





//
// IMPLEMENTATION
//

#ifdef JED_CMD_IMPLEMENTATION

namespace cmd
{

namespace commandPacket
{

	CommandPacket* GetNextCommandPacket(CommandPacket packet)
	{
		return reinterpret_cast<CommandPacket*>(reinterpret_cast<char*>(packet) + OFFSET_NEXT_COMMAND_PACKET);
	}

	BackendDispatchFunction* GetBackendDispatchFunction(CommandPacket packet)
	{
		return reinterpret_cast<BackendDispatchFunction*>(reinterpret_cast<char*>(packet) + OFFSET_BACKEND_DISPATCH_FUNCTION);
	}

	void StoreNextCommandPacket(CommandPacket packet, CommandPacket nextPacket)
	{
		*commandPacket::GetNextCommandPacket(packet) = nextPacket;
	}
	 
	void StoreBackendDispatchFunction(CommandPacket packet, BackendDispatchFunction dispatchFunction)
	{
		*commandPacket::GetBackendDispatchFunction(packet) = dispatchFunction;
	}

	CommandPacket const LoadNextCommandPacket(CommandPacket const packet)
	{
		return *GetNextCommandPacket(packet);
	}

	BackendDispatchFunction const LoadBackendDispatchFunction(CommandPacket const packet)
	{
		return *GetBackendDispatchFunction(packet);
	}

	void const* LoadCommand(CommandPacket const packet)
	{
		return reinterpret_cast<char*>(packet) + OFFSET_COMMAND;
	}

}; // namespace commandPacket

}; // namespace cmd

#endif