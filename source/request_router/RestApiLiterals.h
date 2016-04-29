#ifndef PATR_RESPONSE_LITERALS_H
#define PATR_RESPONSE_LITERALS_H

namespace Rest {

namespace Azure {

constexpr auto AZURE_KEY_RELATIVE_PATH = "keys.txt";

}

namespace Endpoint {

constexpr auto SEGMENTATION_ENDPOINT = "/api/segment";

}

namespace Response {

constexpr auto STATUS = "status";
constexpr auto ERROR_DESCRIPTION = "error_description";
constexpr auto SEGMENTATION_COORDINATES = "coordinates";

} // namespace Response

namespace Request {

constexpr auto URL = "url";
constexpr auto ACTION = "action";
constexpr auto SEGMENTATION_ACTION = "Segmentation";

} // namespace Request

} // namespace Rest

#endif // PATR_RESPONSE_LITERALS_H
