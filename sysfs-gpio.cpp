#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cl3/core/io_file.hpp>
#include "sysfs-gpio.hpp"

using namespace cl3::io::stream::fd;
using namespace cl3::io::text;
using namespace cl3::io::file;
using namespace cl3::io::text::string;

#define SYSERR(expr) (([&](){ const auto r = ((expr)); if( (long)r == -1L ) { throw #expr; } else return r; })())

namespace gpio
{
	namespace sysfs
	{
		static void WriteFile(const char* const file, const char* const data, ...)
		{
			va_list l;
			va_start(l, data);

			char __file[256];
			vsnprintf(__file, sizeof(__file), file, l);

			char __data[256];
			vsnprintf(__data, sizeof(__data), data, l);

			va_end(l);

			const int fd = SYSERR(open(__file, O_WRONLY|O_CLOEXEC|O_NOCTTY|O_SYNC));
			try
			{
				const ssize_t len = strnlen(__data, sizeof(__data));
				if(SYSERR(write(fd, __data, len)) != len)
					throw "write failed";
			}
			catch(...)
			{
				close(fd);
				throw;
			}

			close(fd);
		}

		u32_t TPin::ID() const
		{
			return this->id;
		}

		EMode TPin::Mode() const
		{
			return this->mode;
		}

		void TPin::Mode(EMode m)
		{
			if(this->mode == m) return;

			switch(m)
			{
				case EMode::INPUT:
					WriteFile("/sys/class/gpio/gpio%u/mode", "in", this->id);
					break;
				case EMode::OUTPUT:
					WriteFile("/sys/class/gpio/gpio%u/mode", "out", this->id);
					break;
			}

			this->mode = m;
		}

		bool TPin::Value() const
		{
			byte_t chr;
			CL3_CLASS_LOGIC_ERROR(this->f_value.Read(0, &chr, 1) != 1);
			return (char)chr != '0';
		}

		void TPin::Value(bool b)
		{
			CL3_CLASS_LOGIC_ERROR(this->mode != EMode::OUTPUT);
			const byte_t chr = (byte_t)(b ? '1' : '0');
			CL3_CLASS_LOGIC_ERROR(this->f_value.Write(0, &chr, 1) != 1);
		}

		ETrigger TPin::Trigger() const
		{
			return this->trigger;
		}

		void TPin::Trigger(ETrigger t)
		{
			if(this->trigger == t) return;

			switch(t)
			{
				case ETrigger::NONE:
					WriteFile("/sys/class/gpio/gpio%u/edge", "none", this->id);
					break;
				case ETrigger::RISING:
					WriteFile("/sys/class/gpio/gpio%u/edge", "rising", this->id);
					break;
				case ETrigger::FALLING:
					WriteFile("/sys/class/gpio/gpio%u/edge", "falling", this->id);
					break;
			}

			this->trigger = t;
		}

		TWaitable TPin::OnTrigger() const
		{
			return this->f_value.FD().OnInputReady();
		}

		TPin::~TPin()
		{
			WriteFile("/sys/class/gpio/unexport", "%u", this->id);
		}

		TPin::TPin(const u32_t id) : id(id), mode(EMode::INPUT), trigger(ETrigger::NONE)
		{
			WriteFile("/sys/class/gpio/export", "%u", this->id);
			WriteFile("/sys/class/gpio/gpio%u/direction", "in", this->id);
			WriteFile("/sys/class/gpio/gpio%u/edge", "none", this->id);
			this->f_value = TFile(TString("/sys/class/gpio/gpio") + id + "/value", TAccess::RW);
		}

		TList<u32_t> TController::AvailablePins() const
		{
			byte_t buffer[16];

			memset(buffer, 0, sizeof(buffer));
			TFile(this->sysfs_path + "base").Read(0, buffer, sizeof(buffer) - 1);
			const u32_t base = (u32_t)atol((const char*)buffer);

			memset(buffer, 0, sizeof(buffer));
			TFile(this->sysfs_path + "ngpio").Read(0, buffer, sizeof(buffer) - 1);
			const u32_t ngpio = (u32_t)atol((const char*)buffer);

			TList<u32_t> ids;
			for(u32_t i = 0; i < ngpio; i++)
				ids.Append(base + i);

			return ids;
		}

		IPin* TController::Claim(const u32_t id)
		{
			return new TPin(id);
		}

		TController::TController(const char* const sysfs_path) : sysfs_path( TString(sysfs_path) + (TString(sysfs_path)[-1] == '/' ? "" : "/") )
		{
		}

		TController::~TController()
		{
		}
	};
}
