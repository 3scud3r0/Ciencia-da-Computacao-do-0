import { AbsoluteFill, useCurrentFrame, spring, useVideoConfig, interpolate } from "remotion";

const ITEMS = [
  "Raw socket frame sender",
  "TCP state machine",
  "TLS 1.3 handshake",
  "Userspace TCP/IP stack",
  "eBPF programs",
  "CNI plugin",
];

export const BuildListScene: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();

  const titleOpacity = interpolate(frame, [0, 10], [0, 1], {
    extrapolateRight: "clamp",
  });

  return (
    <AbsoluteFill
      style={{
        backgroundColor: "#0d1117",
        justifyContent: "center",
        alignItems: "center",
        padding: 60,
      }}
    >
      <div
        style={{
          fontSize: 32,
          fontWeight: 600,
          fontFamily: "'Inter', sans-serif",
          color: "#8b949e",
          marginBottom: 40,
          opacity: titleOpacity,
        }}
      >
        What you build
      </div>

      <div style={{ display: "flex", flexDirection: "column", gap: 16, width: 700 }}>
        {ITEMS.map((item, i) => {
          const delay = 10 + i * 18;
          const progress = spring({
            frame: frame - delay,
            fps,
            config: { damping: 15, stiffness: 80 },
          });

          const x = interpolate(progress, [0, 1], [-60, 0]);
          const opacity = interpolate(progress, [0, 0.4], [0, 1], {
            extrapolateRight: "clamp",
          });

          return (
            <div
              key={i}
              style={{
                display: "flex",
                alignItems: "center",
                gap: 20,
                opacity,
                transform: `translateX(${x}px)`,
              }}
            >
              <div
                style={{
                  width: 10,
                  height: 10,
                  borderRadius: "50%",
                  backgroundColor: "#00897B",
                  flexShrink: 0,
                }}
              />
              <span
                style={{
                  fontSize: 34,
                  fontWeight: 600,
                  fontFamily: "'JetBrains Mono', monospace",
                  color: "#e6edf3",
                }}
              >
                {item}
              </span>
            </div>
          );
        })}
      </div>
    </AbsoluteFill>
  );
};
