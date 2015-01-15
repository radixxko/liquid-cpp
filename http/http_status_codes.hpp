#ifndef XCLBR_HTTP_STATUS_CODES_HPP
#define XCLBR_HTTP_STATUS_CODES_HPP

namespace xclbr
{
	namespace http
	{
		enum http_status_code
		{
			unknown = 0,

			continue_code = 100,
			switching_protocols = 101,

			ok = 200,
			created = 201,
			accepted = 202,
			non_authoritative_information = 203,
			no_content = 204,
			reset_content = 205,
			partial_content = 206,

			multiple_choices = 300,
			moved_permanently = 301,
			found = 302,
			see_other = 303,
			not_modified = 304,
			use_proxy = 305,
			temporary_redirect = 307,

			bad_request = 400,
			unauthorized = 401,
			payment_required = 402,
			forbidden = 403,
			not_found = 404,
			method_not_allowed = 405,
			not_acceptable = 406,
			proxy_authentication_required = 407,
			request_timeout = 408,
			conflict = 409,
			gone = 410,
			length_required = 411,
			precondition_failed = 412,
			request_entity_too_large = 413,
			request_uri_too_long = 414,
			unsupported_media_type = 415,
			request_range_not_satisfiable = 416,
			expectation_failed = 417,
			im_a_teapot = 418,
			upgrade_required = 426,
			precondition_required = 428,
			too_many_requests = 429,
			request_header_fields_too_large = 431,

			internal_server_error = 500,
			not_implemented = 501,
			bad_gateway = 502,
			service_unavailable = 503,
			gateway_timeout = 504,
			http_version_not_supported = 505,
			not_extended = 510,
			network_authentication_required = 511
		};
	}
}

#endif // XCLBR_HTTP_STATUS_CODES_HPP
