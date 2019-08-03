#ifndef TEMPFILEREAD_H
#define TEMPFILEREAD_H

template< class DataEntryType, Mem::MEMORY_CHUNK Chunk, class HeaderType>
class CScopedFileRead
{
public:
	CScopedFileRead(const ntstd::String& fullPath)
		: fileData_(NULL)
		, numElements_(0)
	{
		if (ntStr::IsNull(fullPath))
		{
			return;
		}

		if (!File::Exists(ntStr::GetString(fullPath)))
		{
			return;
		}

		File	dataFile(ntStr::GetString(fullPath), File::FT_READ | File::FT_BINARY);

		unsigned int size = dataFile.GetFileSize();

		int numElems = (size - sizeof(HeaderType)) / sizeof(DataEntryType);

		if (numElems <= 0)
		{
			return;
		}

		dataFile.Read(&header_, sizeof(HeaderType));

		fileData_ = NT_NEW_ARRAY_CHUNK(Chunk) DataEntryType[numElems];
		dataFile.Read(fileData_, numElems * sizeof(DataEntryType));

		numElements_ = numElems;

	}

	~CScopedFileRead()
	{
		NT_DELETE_ARRAY_CHUNK(SPEEDGRASS_MEMORY_CHUNK, fileData_);
		numElements_ = 0;
	}

	bool IsValid() const
	{
		return 0 != numElements_;
	}

	size_t GetNumElements() const
	{
		return numElements_;
	}

	const HeaderType&	GetHeader() const
	{
		return header_;
	}

	DataEntryType& operator [] (size_t index)
	{
		ntError(index < numElements_);
		return fileData_[index];
	}

	DataEntryType*	GetData()
	{
		return fileData_;
	}

private:
	DataEntryType*	fileData_;
	HeaderType		header_;
	size_t			numElements_;
};

#endif
