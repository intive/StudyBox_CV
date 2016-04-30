#ifndef PATR_RESPONSE_LITERALS_H
#define PATR_RESPONSE_LITERALS_H

namespace Rest {

    namespace Endpoint {

    constexpr auto SEGMENTATION_ENDPOINT = "/api/segment";
    constexpr auto TEXT_ANALYSIS_ENDPOINT = "/api/analysis";

    } // namespace Endpoint

    namespace Response {

    constexpr auto STATUS = "status";
    constexpr auto ERROR_DESCRIPTION = "error_description";

    constexpr auto TEXT_ANALYSIS_RESULTS = "results";

    constexpr auto SEGMENTATION_COORDINATES = "coordinates";

        namespace ErrorStrings {

            constexpr auto BAD_ACTION = "unrecognised action for current api";
            constexpr auto BAD_JSON_TYPE = "request body contains invalid field types";
            constexpr auto UNKNOWN_JSON_FIELDS = "server could not handle request, invalid fields specified or possibly unsupported syntax";
            constexpr auto BAD_JSON = "request body could not be read as valid json";
            constexpr auto UNKNOWN_ELABORATE = "server could not handle request, reason: ";
            constexpr auto UNKNOWN = "server could not handle request, error unkown";
            constexpr auto BAD_IMAGE = "invalid or unsupported image format";

        }

    } // namespace Response

    namespace Request {

        constexpr auto URL = "url";
        constexpr auto ACTION = "action";

        constexpr auto TEXT_ANALYSIS_TEXT_FOR_ANALYSIS = "text_for_analysis";
        constexpr auto SEGMENTATION_ACTION = "Segmentation";

    } // namespace Request

} // namespace Rest

#endif // PATR_RESPONSE_LITERALS_H
