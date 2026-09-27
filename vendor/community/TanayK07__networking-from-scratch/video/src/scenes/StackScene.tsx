import { AbsoluteFill, useCurrentFrame, spring, useVideoConfig, interpolate } from "remotion";

const LAYERS = [
  { name: "HTTP", color: "#e06c75", phase: "Phase 6" },
  { name: "TLS 1.3", color: "#c678dd", phase: "Phase 7" },
  { name: "TCP", color: "#61afef", phase: "Phase 4" },
  { name: "IP", color: "#56b6c2", phase: "Phase 3" },
  { name: "Ethernet", color: "#98c379", phase: "Phase 2" },
  { name: "Bits & Wires", color: "#d19a66", phase: "Phase 1" },
];

export const StackScene: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();

  const titleOpacity = interpolate(frame, [0, 15], [0, 1], {
    extrapolateRight: "clamp",
  });

  return (
    <AbsoluteFill
      style={{
        backgroundColor: "#0d1117",
        justifyContent: "center",
        alignItems: "center",
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
        Build every layer
      </div>

      <div style={{ display: "flex", flexDirection: "column", gap: 8, width: 700 }}>
        {LAYERS.map((layer, i) => {
          const reverseIndex = LAYERS.length - 1 - i;
          const delay = reverseIndex * 20 + 15;

          const slideProgress = spring({
            frame: frame - delay,
            fps,
            config: { damping: 14, stiffness: 60 },
          });

          const x = interpolate(slideProgress, [0, 1], [800, 0]);
          const opacity = interpolate(slideProgress, [0, 0.3], [0, 1], {
            extrapolateRight: "clamp",
          });

          const glowOpacity = interpolate(
            frame - delay - 20,
            [0, 15, 30],
            [0, 0.6, 0],
            {
              extrapolateLeft: "clamp",
              extrapolateRight: "clamp",
            }
          );

          return (
            <div
              key={layer.name}
              style={{
                transform: `translateX(${x}px)`,
                opacity,
                position: "relative",
              }}
            >
              <div
                style={{
                  backgroundColor: layer.color + "18",
                  border: `2px solid ${layer.color}60`,
                  borderRadius: 12,
                  padding: "18px 30px",
                  display: "flex",
                  justifyContent: "space-between",
                  alignItems: "center",
                  boxShadow: `0 0 ${20 * glowOpacity}px ${layer.color}40`,
                }}
              >
                <span
                  style={{
                    fontSize: 32,
                    fontWeight: 700,
                    fontFamily: "'JetBrains Mono', monospace",
                    color: layer.color,
                  }}
                >
                  {layer.name}
                </span>
                <span
                  style={{
                    fontSize: 20,
                    fontFamily: "'Inter', sans-serif",
                    color: "#8b949e",
                  }}
                >
                  {layer.phase}
                </span>
              </div>
            </div>
          );
        })}
      </div>
    </AbsoluteFill>
  );
};
