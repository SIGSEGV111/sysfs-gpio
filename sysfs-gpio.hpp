#ifndef __sysfs_gpio_sysfs_gpio_hpp__
#define __sysfs_gpio_sysfs_gpio_hpp__

#include "gpio.hpp"
#include <cl3/core/io_file.hpp>
#include <cl3/core/io_text_string.hpp>

namespace gpio
{
	namespace sysfs
	{
		class TPin : public IPin
		{
			protected:
				const u32_t id;
				EMode mode;
				ETrigger trigger;
				cl3::io::file::TFile f_value;

			public:
				u32_t ID() const  final override CL3_GETTER;

				EMode Mode() const final override CL3_GETTER;
				void Mode(EMode) final override CL3_SETTER;

				bool Value() const final override;
				void Value(bool) final override CL3_SETTER;

				ETrigger Trigger() const final override CL3_GETTER;
				void Trigger(ETrigger) final override CL3_SETTER;

				cl3::io::stream::fd::TWaitable OnTrigger() const final override;

				~TPin();
				TPin(const u32_t id);
		};

		class TController : public IController
		{
			protected:
				const cl3::io::text::string::TString sysfs_path;

			public:
				TList<u32_t> AvailablePins() const final override;
				IPin* Claim(const u32_t id) final override;

				TController(const char* const sysfs_path);
				~TController();
		};
	};
}

#endif
