
#include <hkbase/hkBase.h>
#include <hkbase/stream/hkSocket.h>

class ntPS3Socket : public hkSocket
{
	public:
		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_BASE_CLASS );

		typedef int socket_t;

		ntPS3Socket(socket_t s=socket_t(-1));

		virtual ~ntPS3Socket();

		virtual hkBool isOk() const;

		virtual void close();

		virtual int read( void* buf, int nbytes);

		virtual int write( const void* buf, int nbytes);

		// client

		virtual hkResult connect(const char* servername, int portNumber);

		
		// server

		hkResult listen(int port);
		hkSocket* pollForNewClient();

		static void shutdownNetwork();
		
		static void ReplaceSocketImpl();

	protected:

		hkResult createSocket();

		socket_t m_socket;
	};
