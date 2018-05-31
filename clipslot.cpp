#include "stdafx.h"
#include "clipslot.h"
Clipslot _clipslot;

std::vector<HWND> Clipslot::slotButtonList;

// Hàm sắp xếp clippiece theo số thứ tự logic
struct sortBySlot {
	inline bool operator() (Clippiece* cl1, Clippiece* cl2) {
		return (cl1->getPosition() < cl2->getPosition());
	}
};

Clipslot::Clipslot() {
	clippieceCount = 0;
	hotkeySlotCount = 0;
	requestClippiecePosition = -1;
	currentUniqueId = 0;
}

Clipslot::~Clipslot() {
	std::vector<Clippiece*>::iterator it;
	for (it = clippieceVector.begin(); it != clippieceVector.end(); it++) {
		delete *it;
		*it = NULL;
	}
	clippieceVector.clear();

	std::map<DWORD, std::pair<DWORD, char*> >::iterator mit;
	for (mit = sharingMap.begin(); mit != sharingMap.end(); mit++) {
		free(mit->second.second);
		mit->second.second = NULL;
	}
	sharingMap.clear();
}

int Clipslot::size()		{	return clippieceCount;			}
int Clipslot::capacity()	{	return clippieceVector.size();	}

void Clipslot::setSlotButtonList(HWND button)		{	slotButtonList.push_back(button);					}
HWND Clipslot::getSlotButton(DWORD selectedButton)	{	return Clipslot::slotButtonList[selectedButton];	}


int Clipslot::addToClipslot(Clippiece *addingClippiece, DWORD position) {
	try {
		if (addingClippiece->countFormat() <= 0)	// Từ chối nếu không có format nào
			throw CLIPSLOT_ERROR_EMPTY_CLIPPIECE;

		bool isAffectHotkey = true;
		DWORD selectedPosition = position;

		if (selectedPosition == CLIPPIECE_NOSLOT) {
			// Tìm vị trí hotkey còn trống
			FOR(hotkeySlotCount) {
				consoleOutput(cnt, CONSOLE_SPACE);
				if (clippieceVector[cnt] == NULL) {
					selectedPosition = cnt;
					break;
				}
				if (clippieceVector[cnt]->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD) {
					selectedPosition = cnt;
					break;
				}
			}
			// Nếu không tìm thấy mặc định thêm vào cuối dãy
			if (selectedPosition == CLIPPIECE_NOSLOT) {
				selectedPosition = clippieceCount;
				if (hotkeySlotCount < MAX_HOTKEY_COUNT) {
					hotkeySlotCount += 1;
					isAffectHotkey = true;
				}
				else
					isAffectHotkey = false;
				clippieceVector.push_back(NULL);
			}
		}

		clippieceCount++;
		if (clippieceVector[selectedPosition] == NULL) {
			// Vị trí thêm vào trống (NULL) thì không xử lý gì thêm
		}
		else if (clippieceVector[selectedPosition]->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD) {
			// Vị trí thêm vào chứa clippiece được đánh dấu là sẵn sàng để thay thế thì ghi đè an toàn
			delete clippieceVector[selectedPosition];	// Ghi đè
		}
		else {
			// Trường hợp còn lại: Vị trí thêm vào chứa clippiece vẫn còn hoạt động
			// Trong trường hợp này diễn ra sự thay thế, vậy nên không làm tăng số lượng clippiece
			clippieceCount--;
		}

		addingClippiece->setPosition(selectedPosition, isAffectHotkey);
		clippieceVector[selectedPosition] = addingClippiece;
		
		// Sắp xếp lại theo thức tự logic (không cần thiết, nhưng vẫn thực hiện)
		std::sort(clippieceVector.begin(), clippieceVector.end(), sortBySlot());
		return selectedPosition;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_EMPTY_CLIPPIECE:
			delete addingClippiece;
			addingClippiece = NULL;
			consoleOutput(_T("Clipslot adding failed - No data in clippiece"), CONSOLE_NEWLINE);
			break;
		case CLIPSLOT_ERROR_OCCUPIEDSLOT:
			consoleOutput(_T("Clipslot adding failed - The position is occupied"), CONSOLE_NEWLINE);
			break;
		}
		return CLIPPIECE_NOSLOT;
	}
}

Clippiece * Clipslot::getClippiece(DWORD position) {
	try {
		if (position < 0
		|| position >= clippieceVector.size()
		|| position == CLIPPIECE_NOSLOT)
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		return clippieceVector[position];
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_OUT_OF_RANGE:
			consoleOutput(_T("Get clippiece failed - Out of range at position"), CONSOLE_SPACE);
			consoleOutput(position, CONSOLE_NEWLINE);
			break;
		}
		return NULL;
	}
}
bool Clipslot::setClippiece(Clippiece *addingClippiece, DWORD position) {
	try {
		// Cẩn thận! Phần tử bị thay thế sẽ KHÔNG bị hủy tự động
		if (position < 0
		|| position >= clippieceVector.size())
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		clippieceVector[position] = addingClippiece;
		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_OUT_OF_RANGE:
			consoleOutput(_T("Set clippiece failed - Out of range at position"), CONSOLE_SPACE);
			consoleOutput(position, CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
}

Clippiece * Clipslot::getLastClippiece()			{	return clippieceVector.back();							}
// Cẩn thận! Phần tử bị thay thế sẽ KHÔNG bị hủy tự động
void Clipslot::setLastClippiece(Clippiece* target)	{	clippieceVector[clippieceVector.size() - 1] = target;	}

bool Clipslot::swapClippiece(DWORD position1, DWORD position2) {
	try {
		if (position1 < 0
			|| position1 >= clippieceVector.size())
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		if (position2 < 0
			|| position2 >= clippieceVector.size())
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		// Hoán đổi vị trí logic
		if(getClippiece(position1))
			getClippiece(position1)->setPosition(position2, position2 < MAX_HOTKEY_COUNT ? true : false);
		if (getClippiece(position2))
			getClippiece(position2)->setPosition(position1, position1 < MAX_HOTKEY_COUNT ? true : false);

		// Hoán đổi vị trí vật lý
		Clippiece *temp = clippieceVector[position2];
		clippieceVector[position2] = clippieceVector[position1];
		clippieceVector[position1] = temp;

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_OUT_OF_RANGE:
			consoleOutput(_T("Swap clippiece failed - Out of range positions"), CONSOLE_SPACE);
			consoleOutput(position1, CONSOLE_SPACE);
			consoleOutput(position2, CONSOLE_NEWLINE);
			break;
		}
		return false;
	}
}

// ==========
bool Clipslot::removeClippiece(DWORD removingPosition) {
	Clippiece *clippieceToRemove = getClippiece(removingPosition);
	try {
		if (clippieceToRemove == NULL)
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		// Hủy trạng thái chia sẻ nếu có
		if (clippieceToRemove->getSharingStatus() == CLIPPIECE_STATUS_IS_SHARING) {
			toggleShareSlot(removingPosition);
			clippieceToRemove->setSharingStatus(CLIPPIECE_STATUS_NO_SHARE);
		}

		if (0 <= removingPosition
		&& removingPosition < MAX_HOTKEY_COUNT) {
			// Không thay đổi các clippiece đã được đánh dấu là chờ ghi đè, chúng có thể được ghi đè an toàn
			// và các tài nguyên của chúng đều đã giải phóng hết
			if (clippieceToRemove->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)
				return true;
			else {
				// Nếu clippiece này nằm trong khoản chứa hotkey, chỉ đánh dấu cho phép ghi đè thay vì xóa hẳn
				clippieceToRemove->setStatus(CLIPPIECE_STATUS_READY_TO_DISCARD);
				// Giải phóng các tài nguyên của clippiece này
				clippieceToRemove->destroyData();
				clippieceToRemove->removeAllButton();
				// Việc này thay đổi số lượng phần tử hữu dụng
				clippieceCount -= 1;
				throw CLIPSLOT_DISCARD;
			}
		}

		// Kỹ thuật xóa: Đổi chỗ dữ liệu ở vị trí bị xóa với phần tử cuối cùng của kho chứa
		DWORD lastPosition = capacity() - 1;
		swapClippiece(lastPosition, removingPosition);

		// Việc này thay đổi số lượng phần tử hữu dụng
		clippieceCount -= 1;

		// sau đó loại bỏ phần tử cuối cùng với độ phức tạp O(1)
		delete getLastClippiece();
		clippieceVector.pop_back();

		consoleOutput(_T("Successfully removed clippiece at position"), CONSOLE_SPACE);
		consoleOutput(removingPosition, CONSOLE_NEWLINE);

		return true;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_DISCARD:
			consoleOutput(_T("Remove clippiece - Instead discard clippiece at slot"), CONSOLE_SPACE);
			break;
		case CLIPSLOT_ERROR_OUT_OF_RANGE:
			consoleOutput(_T("Remove clippiece failed - No clippiece to remove at slot"), CONSOLE_SPACE);
			break;
		}
		consoleOutput(removingPosition, CONSOLE_NEWLINE);
		return false;
	}
}

bool Clipslot::requestChangeClippiecePosition(DWORD selectedSlot) {
	Clippiece *requestClippiece = getClippiece(selectedSlot);
	if (!(requestClippiece == NULL
	|| requestClippiece->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)) {
		requestClippiecePosition = selectedSlot;
		return true;
	}
	return false;
}
void Clipslot::cancelChangeClippiecePosition(DWORD selectedSlot)	{	requestClippiecePosition = MAX_HOTKEY_COUNT;	}
DWORD Clipslot::getRequestClippiecePosition()						{	return requestClippiecePosition;				}
DWORD Clipslot::changeClippiecePosition(DWORD selectedPosition) {
	try {
		Clippiece *destinationClippiece = getClippiece(selectedPosition);

		if (selectedPosition < 0
		|| selectedPosition >= clippieceVector.size()
		|| selectedPosition == CLIPPIECE_NOSLOT)
			throw CLIPSLOT_ERROR_OUT_OF_RANGE;

		consoleOutput(_T("Change position to"), CONSOLE_SPACE);
		consoleOutput(selectedPosition, CONSOLE_NEWLINE);

		if (destinationClippiece == NULL
		|| destinationClippiece->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD) {
			// Vị trí được dời tới đang chờ ghi đè
			if (swapClippiece(requestClippiecePosition, selectedPosition)) {
				removeClippiece(requestClippiecePosition);
				int newPosition = addToClipslot(getClippiece(selectedPosition), selectedPosition);

				// Nếu clippiece ở ngoài khoảng có phím tắt đi vào khoảng có phím tắt
				// thì phải bù lại một clippiece
				// vì clippiece chờ ghi đè khi bị đưa ra ngoài khoảng không có phím tắt
				// sẽ bị hệ thống xóa tự động
				if (requestClippiecePosition >= MAX_HOTKEY_COUNT)
					clippieceCount += 1;

				return newPosition;
			}
			return CLIPPIECE_NOSLOT;
		}
		else {
			// Vị trí được dời tới đã bị chiếm
			if (swapClippiece(requestClippiecePosition, selectedPosition)) {
				int newPosition = addToClipslot(getClippiece(selectedPosition), selectedPosition);
				return newPosition;
			}
			return CLIPPIECE_NOSLOT;
		}
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_OUT_OF_RANGE:
			consoleOutput(_T("Clippiece change position failed - Destination out of range at"), CONSOLE_SPACE);
			consoleOutput(selectedPosition, CONSOLE_NEWLINE);
			break;
		}
		return CLIPPIECE_NOSLOT;
	}
}

Clippiece* Clipslot::triggerClippieceSlot(DWORD selectedPosition) {
	Clippiece *targetClippiece = getClippiece(selectedPosition);
	try {
		if (targetClippiece == NULL
			|| targetClippiece->getStatus() != CLIPPIECE_STATUS_READY_TO_USE
			|| targetClippiece->getSharingStatus() == CLIPPIECE_STATUS_IS_SHARED)
			throw CLIPSLOT_ERROR_EMPTYSLOT;

		consoleOutput(_T("Attempt to trigger clippiece at slot "), CONSOLE_NORMAL);
		consoleOutput(selectedPosition, 1);

		targetClippiece->injectAll();
		return targetClippiece;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_EMPTYSLOT:
			consoleOutput(_T("Clipslot trigger failed - Unoccupied slot"), CONSOLE_NEWLINE);
			break;
		}
		return NULL;
	}
}

int Clipslot::createClippieceFromCurrentClipboard() {
	try {
		if (!OpenClipboard(Window::globalhWnd))
			throw CLIPBOARD_ERROR_NOT_OPEN;

		if (GetClipboardData(Window::CF_YCLIP) != NULL)
			throw YCLIP_ERROR_RECURSIVE;

		Clippiece* clp = new Clippiece();
		if (clp == NULL)
			throw CLIPPIECE_ERROR_INIT_FAILED;

		UINT uFormat = EnumClipboardFormats(0);		// Lấy format khả dụng đầu tiên trong clipboard
		HANDLE hContent;
		while (uFormat) {
			HGLOBAL hContentCopy = NULL;
			HGLOBAL hDataLock = NULL;
			try {
				hContent = GetClipboardData(uFormat);

				if (hContent) {
					// Chèn dữ liệu
					switch (uFormat) {
					case CF_BITMAP:
					case CF_METAFILEPICT:
					case CF_ENHMETAFILE: {
						// Các format không xử lý
						hContentCopy = NULL;
					} break;
					case CF_DIBV5: {
						// Format ảnh
						PBITMAPV5HEADER bitmapInfo = (PBITMAPV5HEADER)hContent;
						DWORD bmSize = bitmapSizeCalculateV5(bitmapInfo);

						hContentCopy = GlobalAlloc(GPTR, bmSize);
						if (hContentCopy == NULL)
							throw ALLOCATION_FAILED;

						memcpy(hContentCopy, hContent, bmSize);
					} break;
					case CF_DIB: {
						PBITMAPINFO bitmapInfo = (PBITMAPINFO)hContent;
						DWORD bmSize = bitmapSizeCalculate(bitmapInfo);

						hContentCopy = GlobalAlloc(GPTR, bmSize);
						if (hContentCopy == NULL)
							throw ALLOCATION_FAILED;

						memcpy(hContentCopy, hContent, bmSize);
					} break;
					default: {
						// Format tệp / xâu ký tự
						hDataLock = GlobalLock(hContent);

						if (hDataLock == NULL)
							throw GLOBALLOCK_FAILED;

						// Các trường hợp còn lại
						SIZE_T dataSize = GlobalSize(hContent);
						hContentCopy = GlobalAlloc(GPTR, dataSize);
						if (hContentCopy == NULL)
							throw ALLOCATION_FAILED;

						memcpy(hContentCopy, hDataLock, dataSize);
					}
					}

					clp->insertData(uFormat, hContentCopy);
					GlobalUnlock(hContent);
				}
				else {
					consoleOutput(_T("Clipboard change event - Warning: Failed to get clipboard data with format "), 0);
					consoleOutput(uFormat, 1);
				}
			}
			catch (int error) {
				switch (error) {
				case GLOBALLOCK_FAILED:
					consoleOutput(_T("Clipboard change event - Failed to lock data"), CONSOLE_NEWLINE);
					break;
				case ALLOCATION_FAILED:
					consoleOutput(_T("Clipboard change event - Allocation data failed"), CONSOLE_NEWLINE);
					break;
				case UNSUPPORTED_FORMAT:
					consoleOutput(_T("Clipboard change event - Unsupported format"), CONSOLE_SPACE);
					consoleOutput(uFormat, CONSOLE_NEWLINE);
					break;
				}

				if (hDataLock)
					GlobalUnlock(hContent);
			}
			uFormat = EnumClipboardFormats(uFormat);	// Lấy format tiếp theo
		}
		CloseClipboard();			// Luôn luôn đóng clipboard

		// Thêm mô tả
		clp->setDescription();

		// Đánh ID duy nhất
		clp->setUniqueId(currentUniqueId);
		currentUniqueId += 1;

		return addToClipslot(clp);
	}
	catch (int error) {
		switch (error) {
		case CLIPBOARD_ERROR_NOT_OPEN:
			consoleOutput(_T("Clipboard change event - Unable to open clipboard"), CONSOLE_NEWLINE);
			break;
		case ALLOCATION_FAILED:
			consoleOutput(_T("Clipboard change event - Allocation clippiece failed"), CONSOLE_NEWLINE);
			break;
		case YCLIP_ERROR_RECURSIVE:
			consoleOutput(_T("Clipboard change event - Recursive adding prevented"), CONSOLE_NEWLINE);
			break;
		}
		CloseClipboard();
		return CLIPPIECE_NOSLOT;
	}
	return CLIPPIECE_NOSLOT;
}
int Clipslot::createClippieceFromSharedDescription(DWORD size, DWORD position, char* content) {
	try {
		// Tạo ra mô tả từ dữ liệu cung cấp
		wchar_t wbuffer[150];	// Lấy dư, kích thước tối đa của phần mô tả chỉ có 100 ký tự wchar_t
		memcpy(wbuffer, content, size);
		wbuffer[size * sizeof(char) / sizeof(wchar_t)] = '\0';
		std::basic_string<wchar_t> wdes(&wbuffer[0], size * sizeof(char) / sizeof(wchar_t));

		// Tạo clippiece mới
		Clippiece *clp = new Clippiece(wdes);
		if (clp == NULL)
			throw CLIPPIECE_ERROR_INIT_FAILED;

		// Lưu lại vị trí được cung cấp cho một format riêng
		clp->insertData(Window::CF_LOGICAL_POSITION, (HGLOBAL)(position + CLIPSLOT_LOGICAL_PADDING));
		clp->setSharingStatus(CLIPPIECE_STATUS_IS_SHARED);

		// Đánh ID duy nhất
		clp->setUniqueId(currentUniqueId);
		currentUniqueId += 1;

		return addToClipslot(clp);
	}
	catch (int error) {
		switch (error) {
		case ALLOCATION_FAILED:
			consoleOutput(_T("Clipslot create from shared description - Allocation clippiece failed"), CONSOLE_NEWLINE);
			break;
		}
		return CLIPPIECE_NOSLOT;
	}
}
int Clipslot::addSharedDataToClippiece(DWORD totalSize, char* data, DWORD position) {
	HGLOBAL hDataLock;
	try {
		Clippiece* clp = getClippiece(position);
		// Phân giải
		// Bỏ qua DWORD đầu tiên (chứa kích thước của gói dữ liệu đến)
		DWORD iterator = sizeof(DWORD);
		size_t subsize;
		size_t dataSize;
		UINT format;
		while (iterator < totalSize) {
			subsize = makeDWORD(data[0 + iterator], data[1 + iterator], data[2 + iterator], data[3 + iterator]);
			iterator += sizeof(DWORD);

			format = makeDWORD(data[0 + iterator], data[1 + iterator], data[2 + iterator], data[3 + iterator]);
			iterator += sizeof(DWORD);

			dataSize = (subsize - sizeof(DWORD) - sizeof(DWORD));
			HGLOBAL hData = GlobalAlloc(GPTR, dataSize);
			if (hData == NULL)
				throw ALLOCATION_FAILED;

			hDataLock = GlobalLock(hData);
			if (hDataLock == NULL)
				throw GLOBALLOCK_FAILED;

			memcpy(hData, data + iterator, dataSize);
			clp->insertData(format, hData);
			iterator += dataSize;
		}
		free(data);
		clp->removeData(Window::CF_LOGICAL_POSITION);

		// Thay đổi các trạng thái của clippiece
		clp->setStatus(CLIPPIECE_STATUS_READY_TO_USE);
		clp->setSharingStatus(CLIPPIECE_STATUS_NO_SHARE);

		clp->removeDownloadButton();

		return position;
	}
	catch (int error) {
		switch (error) {
		case ALLOCATION_FAILED:
			consoleOutput(_T("Clipslot create from shared data failed - Allocation clippiece failed"), CONSOLE_NEWLINE);
			break;
		case GLOBALLOCK_FAILED:
			consoleOutput(_T("Clipslot create from shared data failed - Global lock clippiece failed"), CONSOLE_NEWLINE);
			break;
		}
		free(data);
		return CLIPPIECE_NOSLOT;
	}
}

void Clipslot::toggleShareSlot(DWORD position) {
	Clippiece* target = getClippiece(position);
	try {
		if (target == NULL)
			throw CLIPSLOT_ERROR_EMPTYSLOT;

		if (target->getStatus() == CLIPPIECE_STATUS_READY_TO_DISCARD)
			throw CLIPSLOT_ERROR_EMPTYSLOT;

		if (target->toggleSharingStatus() == CLIPPIECE_STATUS_IS_SHARING) {
			// Nếu kiểm tra đúng trạng thái thì thêm mô tả vào kho chứa mô tả
			DWORD bufferSize;
			char* buffer = target->getSharingDescription(&bufferSize);
			if(bufferSize == 0)
				throw CLIPSLOT_ERROR_CLIPPIECE;

			sharingMap.insert(
				std::pair<DWORD, std::pair<DWORD, char*> >(
					position,
					std::pair<DWORD, char*>(bufferSize, buffer)
				)
			);
		}
		else {
			// Ngược lại thì loại mô tả ra khỏi kho chứa
			std::map<DWORD, std::pair<DWORD, char*> >::iterator mapPosition = sharingMap.find(position);
			if (mapPosition != sharingMap.end()) {
				delete mapPosition->second.second;
				sharingMap.erase(position);
			}
		}
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_EMPTYSLOT:
			consoleOutput(_T("Enable sharing failed - No clippiece to enable"), CONSOLE_NEWLINE);
			break;
		case CLIPSLOT_ERROR_CLIPPIECE:
			consoleOutput(_T("Enable sharing faield - Error from clippiece"), CONSOLE_NEWLINE);
			target->toggleSharingStatus();
			break;
		}
	}
}
// Cấu trúc: Độ dài của cả cấu trúc (DWORD) - Các khối dữ liệu
char* Clipslot::collectClippieceDescription(DWORD* totalSize) {
	try {
		// Tính trước kích thước cần cấp phát
		*totalSize = sizeof(DWORD);
		for (std::map<DWORD, std::pair<DWORD, char*> >::iterator mit = sharingMap.begin();
			mit != sharingMap.end();
			mit++) {
			*totalSize += mit->second.first;
		}
		// Bắt đầu cấp phát và sao chép
		char* buffer = (char*)malloc(*totalSize);
		if (buffer == NULL)
			throw ALLOCATION_FAILED;

		insertDWORD(buffer, *totalSize);
		DWORD iterator = sizeof(DWORD);
		for (std::map<DWORD, std::pair<DWORD, char*> >::iterator mit = sharingMap.begin();
			mit != sharingMap.end();
			mit++) {
			int step = mit->second.first;

			memcpy(buffer + iterator, mit->second.second, step);
			iterator += step;
		}

		return buffer;
	}
	catch (int error) {
		switch (error) {
		case ALLOCATION_FAILED:
			consoleOutput(_T("Collect description failed - Allocation failure"), CONSOLE_NEWLINE);
			break;
		}
		return NULL;
	}
}
char* Clipslot::collectClippieceData(DWORD* totalSize, DWORD position) {
	Clippiece* target = getClippiece(position);
	try {
		if (target == NULL)
			throw CLIPSLOT_ERROR_EMPTYSLOT;

		consoleOutput(target->getSharingStatus(), CONSOLE_NEWLINE);
		if (target->getSharingStatus() != CLIPPIECE_STATUS_IS_SHARING)
			throw CLIPSLOT_ERROR_CLIPPIECE;

		char* buffer = target->collectAll(totalSize);
		return buffer;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_EMPTYSLOT:
			consoleOutput(_T("Clipslot collect clippiece data failed - No clippiece"), CONSOLE_NEWLINE);
			break;
		case CLIPSLOT_ERROR_CLIPPIECE:
			consoleOutput(_T("Clipslot collect clippiece data failed - Clippiece is not shared"), CONSOLE_NEWLINE);
			break;
		}
		*totalSize = 0;
		return NULL;
	}
}

void Clipslot::cleanSharedData() {
	for (int selectedSlot = capacity() - 1; selectedSlot >= 0; selectedSlot--) {
		// Phải chạy ngược vì bản chất của việc xóa
		Clippiece* target = getClippiece(selectedSlot);
		if (target == NULL)
			continue;

		if (target->getSharingStatus() != CLIPPIECE_STATUS_IS_SHARED)
			continue;

		removeClippiece(selectedSlot);
	}
}
DWORD Clipslot::getLogicalPositionFromSharedClippiece(DWORD realPosition) {
	try {
		Clippiece* target = getClippiece(realPosition);
		if (target == NULL)
			throw CLIPSLOT_ERROR_EMPTYSLOT;

		if (target->getSharingStatus() != CLIPPIECE_STATUS_IS_SHARED)
			throw CLIPSLOT_ERROR_CLIPPIECE;

		DWORD result = target->getLogicalPosition();
		if (result == CLIPPIECE_NOSLOT)
			throw CLIPSLOT_ERROR_CLIPPIECE;

		return result - CLIPSLOT_LOGICAL_PADDING;
	}
	catch (int error) {
		switch (error) {
		case CLIPSLOT_ERROR_EMPTYSLOT:
			consoleOutput(_T("Get logical position failed - Empty clippiece"), CONSOLE_NEWLINE);
			return false;
		case CLIPSLOT_ERROR_CLIPPIECE:
			consoleOutput(_T("Get logical position failed - Clippiece error"), CONSOLE_NEWLINE);
			return false;
		}
		return CLIPPIECE_NOSLOT;
	}
}
