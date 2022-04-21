#include "mime_types.hpp"

using std::string;

namespace https_server {
namespace mime_types {

struct mapping
{
  const char* extension;
  const char* mime_type;
} mappings[] =
{
  { "css", "text/css" },
  { "csv", "text/csv" },
  { "txt", "text/plain" },
  { "htm", "text/html" },
  { "html", "text/html" },
  { "js", "text/javascript" },
  { "vtt", "text/vtt" },
  { "gif", "image/gif" },
  { "jpg", "image/jpeg" },
  { "jpeg", "image/jpeg" },
  { "png", "image/png" },
  { "svg", "image/svg+xml" },
  { "apng", "image/apng" },
  { "avif", "image/avif" },
  { "bmp", "image/bmp" },
  { "webp", "image/webp" },
  { "tif", "image/tiff" },
  { "tiff", "image/tiff" },
  { "ico", "image/vnd.microsoft.icon"},
  { "mp4", "video/mp4" },
  { "mpeg", "video/mpeg" },
  { "mp3", "audio/mp3" },
  { "mpga", "audio/mpga" },
  { "wav", "audio/wav" },
  { "ttf", "font/ttf" },
  { "woff", "font/woff" },
  { "woff2", "font/woff2" },
  { "7z", "application/x-7z-compressed" },
  { "tar", "application/x-tar" },
  { "xml", "application/xml" },
  { "gzip", "application/gzip" },
  { "zip", "application/zip" },
  { "eot", "application/vnd.ms-fontobject" },
  { "pdf", "application/pdf" },
  { "json", "application/json" },
  { "pem", "application/x-pem-file" }
};

std::string extensionToType(const string& extension)
{
    for (mapping m: mappings) {
        if (m.extension == extension) {
    		return m.mime_type;
        }
    }

    return "text/plain";
}

std::string typeToExtension(const string& type)
{
	for (mapping m: mappings) {
		if (m.mime_type == type)
			return m.extension;
	}

	return "txt";
}

} // namespace mime_types
} // namespace https_server
