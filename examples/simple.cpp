
#define JED_CMD_IMPLEMENTATION
#include "cmd/cmd.hpp"

#include <type_traits>
#include <iostream>

struct Print
{
	static cmd::BackendDispatchFunction const DISPATCH_FUNCTION;
	char const* string;
};
static_assert(std::is_pod<Print>::value == true, "Print must be a POD.");

void print(void const* data)
{
	Print const* realdata = reinterpret_cast<Print const*>(data);

	if (realdata->string)
	{
		std::cout << realdata->string << std::endl;
	}
}

cmd::BackendDispatchFunction const Print::DISPATCH_FUNCTION = &print;



int main()
{
	cmd::CommandBucket<int> bucket{16};

	Print* command = bucket.AddCommand<Print>(0, 0);

	command->string = "A command string";
	bucket.Submit();
}