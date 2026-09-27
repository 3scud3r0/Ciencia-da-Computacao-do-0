import { AbsoluteFill, useCurrentFrame, spring, useVideoConfig, interpolate } from "remotion";

export const CapstoneScene: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();

  const scale = spring({
    frame,
    fps,
    config: { damping: 12, stiffness: 40 },
  });

  const glowPulse = Math.sin(frame * 0.12) * 0.3 + 0.7;

  const subtitleOpacity = interpolate(frame, [30, 45], [0, 1], {
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });

  return (
    <AbsoluteFill
      style={{
        backgroundColor: "#0d1117",
        justifyContent: "center",
        alignItems: "center",
        padding: 80,
      }}
    >
      {/* Glow background */}
      <div
        style={{
          position: "absolute",
          width: 500,
          height: 500,
          borderRadius: "50%",
          background: `radial-gradient(circle, rgba(0,137,123,${0.15 * glowPulse}) 0%, transparent 70%)`,
        }}
      />

      <div style={{ transform: `scale(${scale})`, textAlign: "center" }}>
        <div
          style={{
            fontSize: 28,
            fontWeight: 500,
            fontFamily: "'Inter', sans-serif",
            color: "#8b949e",
            marginBottom: 16,
            letterSpacing: 4,
            textTransform: "uppercase",
          }}
        >
          Phase 8 — Capstone
        </div>
        <div
          style={{
            fontSize: 48,
            fontWeight: 800,
            fontFamily: "'Inter', sans-serif",
            color: "#e6edf3",
            lineHeight: 1.3,
            maxWidth: 800,
          }}
        >
          Build a TCP/IP stack that makes{" "}
          <span style={{ color: "#00897B" }}>real HTTPS requests</span>
        </div>
      </div>

      <div
        style={{
          position: "absolute",
          bottom: 160,
          opacity: subtitleOpacity,
          fontSize: 24,
          fontFamily: "'JetBrains Mono', monospace",
          color: "#8b949e",
        }}
      >
        Your code. Your stack. Real traffic.
      </div>
    </AbsoluteFill>
  );
};
