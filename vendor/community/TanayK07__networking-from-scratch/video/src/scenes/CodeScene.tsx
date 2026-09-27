import { AbsoluteFill, useCurrentFrame, interpolate } from "remotion";

const CODE_LINES = [
  { code: "/* Build an Ethernet frame */", color: "#8b949e" },
  { code: "memcpy(out, dst, NFS_ETH_ALEN);", color: "#e6edf3", comment: "// Dst MAC" },
  { code: "memcpy(out + 6, src, NFS_ETH_ALEN);", color: "#e6edf3", comment: "// Src MAC" },
  { code: "uint16_t et = htons(ethertype);", color: "#e6edf3", comment: "" },
  { code: "memcpy(out + 12, &et, 2);", color: "#e6edf3", comment: "// EtherType" },
  { code: "memcpy(out + 14, payload, len);", color: "#e6edf3", comment: "// Payload" },
];

export const CodeScene: React.FC = () => {
  const frame = useCurrentFrame();

  const containerOpacity = interpolate(frame, [0, 10], [0, 1], {
    extrapolateRight: "clamp",
  });

  return (
    <AbsoluteFill
      style={{
        backgroundColor: "#0d1117",
        justifyContent: "center",
        alignItems: "center",
        opacity: containerOpacity,
      }}
    >
      {/* Terminal window */}
      <div
        style={{
          backgroundColor: "#161b22",
          borderRadius: 16,
          border: "1px solid #30363d",
          width: 860,
          overflow: "hidden",
          boxShadow: "0 16px 48px rgba(0,0,0,0.4)",
        }}
      >
        {/* Title bar */}
        <div
          style={{
            padding: "14px 20px",
            display: "flex",
            gap: 8,
            alignItems: "center",
            borderBottom: "1px solid #30363d",
          }}
        >
          <div style={{ width: 12, height: 12, borderRadius: "50%", backgroundColor: "#f85149" }} />
          <div style={{ width: 12, height: 12, borderRadius: "50%", backgroundColor: "#d29922" }} />
          <div style={{ width: 12, height: 12, borderRadius: "50%", backgroundColor: "#3fb950" }} />
          <span
            style={{
              marginLeft: 12,
              fontSize: 14,
              fontFamily: "'JetBrains Mono', monospace",
              color: "#8b949e",
            }}
          >
            frame.c
          </span>
        </div>

        {/* Code content */}
        <div style={{ padding: "24px 28px" }}>
          {CODE_LINES.map((line, i) => {
            const delay = 10 + i * 12;
            const opacity = interpolate(frame - delay, [0, 8], [0, 1], {
              extrapolateLeft: "clamp",
              extrapolateRight: "clamp",
            });
            const x = interpolate(frame - delay, [0, 8], [-30, 0], {
              extrapolateLeft: "clamp",
              extrapolateRight: "clamp",
            });

            return (
              <div
                key={i}
                style={{
                  opacity,
                  transform: `translateX(${x}px)`,
                  display: "flex",
                  alignItems: "center",
                  gap: 16,
                  marginBottom: 6,
                }}
              >
                <span
                  style={{
                    fontSize: 16,
                    fontFamily: "'JetBrains Mono', monospace",
                    color: "#8b949e",
                    width: 30,
                    textAlign: "right",
                    flexShrink: 0,
                  }}
                >
                  {i + 1}
                </span>
                <span
                  style={{
                    fontSize: 22,
                    fontFamily: "'JetBrains Mono', monospace",
                    color: i === 0 ? "#8b949e" : "#e6edf3",
                  }}
                >
                  {line.code}
                </span>
                {line.comment && (
                  <span
                    style={{
                      fontSize: 20,
                      fontFamily: "'JetBrains Mono', monospace",
                      color: "#00897B",
                    }}
                  >
                    {line.comment}
                  </span>
                )}
              </div>
            );
          })}
        </div>
      </div>

      {/* Hex bytes floating */}
      <div
        style={{
          position: "absolute",
          bottom: 100,
          display: "flex",
          gap: 10,
          opacity: interpolate(frame, [80, 100], [0, 0.4], {
            extrapolateLeft: "clamp",
            extrapolateRight: "clamp",
          }),
        }}
      >
        {["ff", "ff", "ff", "ff", "ff", "ff", "00", "11", "22", "33", "44", "55", "08", "00"].map(
          (byte, i) => (
            <span
              key={i}
              style={{
                fontSize: 18,
                fontFamily: "'JetBrains Mono', monospace",
                color: i < 6 ? "#98c379" : i < 12 ? "#61afef" : "#e06c75",
              }}
            >
              {byte}
            </span>
          )
        )}
      </div>
    </AbsoluteFill>
  );
};
