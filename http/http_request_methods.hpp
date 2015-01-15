#ifndef XCLBR_HTTP_REQUEST_METHODS_HPP
#define XCLBR_HTTP_REQUEST_METHODS_HPP

#include <assert.h>

namespace xclbr
{
	namespace http
	{
		enum http_request_method
		{
			GET,
			HEAD,
			POST,
			PUT,
			DELETE,
			TRACE,
			OPTIONS,
			CONNECT,
			PATCH
		};

		const char *request_method_to_str(http_request_method m)
		{
			static const char *arr[] =
			{
				"GET",
				"HEAD",
				"POST",
				"PUT",
				"DELETE",
				"TRACE",
				"OPTIONS",
				"CONNECT",
				"PATCH"
			};

			assert((m >= http_request_method::GET) && 
				   (m <= http_request_method::PATCH));

			return arr[(size_t)m];
		}
	}
}

#endif // XCLBR_HTTP_REQUEST_METHODS_HPP
