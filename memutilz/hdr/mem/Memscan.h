#pragma once

namespace mem {
	class Memscan {
		Memdump& _memdump;
	public:
		Memscan() = delete;
		explicit Memscan(Memdump& memdump)
			: _memdump{memdump}
		{}

	};
}