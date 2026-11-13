#include <drogular/job_sse.hpp>

#include <string>
#include <string_view>

namespace drogular {
namespace {

std::string quote(std::string_view input) {
    constexpr char hex[] = "0123456789abcdef";
    std::string out = "\"";
    for (unsigned char c : input) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (c < 0x20) {
                out += "\\u00";
                out += hex[c >> 4];
                out += hex[c & 0x0f];
            } else {
                out += static_cast<char>(c);
            }
        }
    }
    out += '"';

    return out;
}

std::string_view kindName(JobEventKind kind) {
    switch (kind) {
    case JobEventKind::Queued:
        return "job.queued";
    case JobEventKind::Started:
        return "job.started";
    case JobEventKind::Progress:
        return "job.progress";
    case JobEventKind::CancellationRequested:
        return "job.cancellation-requested";
    case JobEventKind::PauseRequested:
        return "job.pause-requested";
    case JobEventKind::Paused:
        return "job.paused";
    case JobEventKind::Resumed:
        return "job.resumed";
    case JobEventKind::Completed:
        return "job.completed";
    case JobEventKind::Failed:
        return "job.failed";
    case JobEventKind::Cancelled:
        return "job.cancelled";
    }

    return "job.unknown";
}

}

std::string encodeJobSse(const JobEvent& event) {
    const auto& job = event.job;
    std::string data = "{\"id\":" + std::to_string(job.id) +
        ",\"name\":" + quote(job.name) +
        ",\"progress\":" + std::to_string(job.progress) +
        ",\"message\":" + quote(job.message) +
        ",\"cancellationRequested\":" +
            (job.cancellationRequested ? "true" : "false") +
        ",\"pauseRequested\":" + (job.pauseRequested ? "true" : "false");
    if (job.failure) {
        data += ",\"failure\":{\"code\":" + quote(job.failure->code) +
                ",\"message\":" + quote(job.failure->message) + "}";
    }
    data += "}";
    return "id: " + std::to_string(event.sequence) + "\nevent: " +
           std::string(kindName(event.kind)) + "\ndata: " + data + "\n\n";
}

std::string encodeJobSseReset() {
    return "event: job.reset\ndata: {\"reason\":\"event_gap\"}\n\n";
}

std::string encodeJobSseHeartbeat() {
    return ": heartbeat\n\n";
}

} // namespace drogular