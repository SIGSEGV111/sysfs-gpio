#ifndef __sysfs_gpio_gpio_hpp__
#define __sysfs_gpio_gpio_hpp__

#include <cl3/core/io_collection_list.hpp>
#include <cl3/core/io_stream_fd.hpp>

namespace gpio
{
	using namespace cl3::io::collection::list;
	using namespace cl3::system::types;

	enum class EMode
	{
		INPUT,
		OUTPUT
	};

	enum class ETrigger
	{
		NONE,
		RISING,
		FALLING
	};

	struct IPin
	{
		virtual u32_t ID() const CL3_GETTER = 0;
		virtual EMode Mode() const CL3_GETTER = 0;
		virtual void Mode(EMode) CL3_SETTER = 0;
		virtual bool Value() const = 0;
		virtual void Value(bool) CL3_SETTER = 0;
		virtual ETrigger Trigger() const CL3_GETTER = 0;
		virtual void Trigger(ETrigger) CL3_SETTER = 0;
		virtual cl3::io::stream::fd::TWaitable OnTrigger() const = 0;
		virtual ~IPin() {}
	};

	struct IController
	{
		// returns a ascending list of all available GPIO pins on this controller
		// MAY contain holes of skipped IDs which are reserved by the system
		virtual TList<u32_t> AvailablePins() const = 0;

		// claims a GPIO pin indentified by ID
		virtual IPin* Claim(const u32_t id) = 0;
	};
}

#endif
