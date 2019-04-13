#ifndef COMMANDPACKET_INL_
#define COMMANDPACKET_INL_

namespace cmd
{

namespace commandPacket
{

	template <typename T>
	size_t GetSize(size_t auxMemorySize)
	{
		return OFFSET_COMMAND + sizeof(T) + auxMemorySize;
	}

	template <typename T>
	CommandPacket Create(size_t auxMemorySize)
	{
		return ::operator new(GetSize<T>(auxMemorySize));
	}

	template <typename T>
	CommandPacket* GetNextCommandPacket(T* command)
	{
		return reinterpret_cast<CommandPacket*>(reinterpret_cast<char*>(command) - OFFSET_COMMAND + OFFSET_NEXT_COMMAND_PACKET);
	}

	template <typename T>
	T* GetCommand(CommandPacket packet)
	{
		return reinterpret_cast<T*>(reinterpret_cast<char*>(packet) + OFFSET_COMMAND);
	}

	template <typename T>
	char* GetAuxiliaryMemory(T* command)
	{
		return reinterpret_cast<char*>(command) + sizeof(T);
	}

	template <typename T>
	void StoreNextCommandPacket(T* command, CommandPacket nextPacket)
	{
		*commandPacket::GetNextCommandPacket<T>(command) = nextPacket;
	}

	template <typename U, typename V>
	U* AppendCommand(V* command, size_t auxMemorySize)
	{
		CommandPacket packet = commandPacket::Create<U>(auxMemorySize);

		// append this command to the given one
		commandPacket::StoreNextCommandPacket<V>(command, packet);

		commandPacket::StoreNextCommandPacket(packet, nullptr);
		commandPacket::StoreBackendDispatchFunction(packet, U::DISPATCH_FUNCTION);

		return commandPacket::GetCommand<U>(packet);
	}

}; // namespace commandPacket

}; // namespace cmd

#endif