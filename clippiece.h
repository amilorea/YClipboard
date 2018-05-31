#pragma once
#include "WindowsProject1.h"

const int CLIPPIECE_DESCRIPTION_SIZE	= 125;	// Kích thước mô tả

const DWORD CLIPPIECE_NOSLOT			= 0x7FFF - 5;	// Vị trí này tương đương với Clippiece trống

const int CLIPPIECE_STATUS_READY_TO_USE		= 0x0;		// Sẵn sàng sử dụng
const int CLIPPIECE_STATUS_READY_TO_DISCARD	= 0x1;		// Chỉ còn giữ chỗ, không sử dụng

const int CLIPPIECE_STATUS_IS_SHARED		= 0x2;		// Dữ liệu được chia sẻ, chưa dùng được
const int CLIPPIECE_STATUS_IS_SHARING		= 0x3;		// Dữ liệu đang được chia sẻ với máy khách
const int CLIPPIECE_STATUS_NO_SHARE			= 0x4;		// Dữ liệu không được chia sẻ

const int FORMAT_ERROR_NO_FILE = 0x99;

const int CLIPPIECE_ERROR_INIT_FAILED		= 0xF00;	// Không khởi tạo được Clippiece
const int CLIPPIECE_ERROR_NO_FORMAT			= 0xF01;	// Không tồn tại format này

class Clippiece {
private:
	std::map<UINT, HGLOBAL> dataMap;		// Map chứa cặp dữ liệu + format
	std::basic_string<wchar_t> des;			// Mô tả
	std::basic_string<wchar_t> waiting;		// Dòng thông báo chờ
	int status;								// Tình trạng khả dụng hiện tại
	int sharingStatus;						// Tình trạng chia sẻ hiện tại
	DWORD slotOccupied;			// Vị trí trong kho chứa
	DWORD uniqueId;				// Id độc nhất

	HWND removeButton;			// Nút xóa
	HWND slotButton;			// Nút sửa phím tắt
	HWND shareButton;			// Nút chia sẻ
	HWND downloadButton;		// Nút tải về

	// Các hàm tạo mô tả
	bool setTimeDescription();				// Mô tả thời gian tạo ra
	bool setFileDescription(bool flag);		// Mô tả tệp
	bool setTextDescription(bool flag);		// Mô tả xâu
	bool setImageDescription(bool flag);	// Mô tả ảnh
	bool setNoneDescription();				// Không có mô tả
public:
	Clippiece();
	Clippiece(std::basic_string<wchar_t> description);
	~Clippiece();
	void setDes(std::basic_string<wchar_t>);
	std::basic_string<wchar_t> getDes();
	void setDescription();		// Thiết lập mô tả
	void toggleWait();			// Thay đổi trạng thái đã copy

	// Các hàm hủy dữ liệu cấp phát động
	void destroyData();
	void removeAllButton();

	// Định danh duy nhất
	void setUniqueId(DWORD id);
	DWORD getUniqueId();
	// Vị trí logic
	void setPosition(DWORD, bool isAffectHotkey = false);
	DWORD getPosition();
	DWORD getLogicalPosition();			// Lấy vị trí trên máy khách của clippiece được chia sẻ

	void setStatus(int status);			// Trạng thái khả dụng
	int getStatus();

	void setSharingStatus(int status);	// Trạng thái chia sẻ
	int getSharingStatus();
	int toggleSharingStatus();

	// Các hàm thêm / lấy / xóa các loại nút bấm
	void setRemoveButton(HWND button);
	HWND getRemoveButton();
	void removeRemoveButton();

	void setSlotButton(HWND button);
	HWND getSlotButton();
	void removeSlotButton();

	void setShareButton(HWND button);
	HWND getShareButton();
	void removeShareButton();

	void setDownloadButton(HWND button);
	HWND getDownloadButton();
	void removeDownloadButton();

	// Các hàm thêm / xóa / kiểm tra dữ liệu ứng với từng format
	bool insertData(UINT format, HGLOBAL data);
	bool removeData(UINT format);
	bool isFormatAvailable(UINT format);
	int countFormat();

	// Đẩy dữ liệu vào clipboard
	bool injectAll();

	// Trả về dữ liệu hoặc mô tả của Clippiece này
	char* collectAll(DWORD* totalSize);
	char* getSharingDescription(DWORD* size);
};