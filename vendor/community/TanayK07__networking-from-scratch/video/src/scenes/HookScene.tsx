import { AbsoluteFill, useCurrentFrame, interpolate, spring, useVideoConfig } from "remotion";

export const HookScene: React.FC = () => {
  const frame = useCurrentFrame();
  const { fps } = useVideoConfig();

  const words = "What if you could build the entire network stack from scratch?".split(" ");

  return (
    <AbsoluteFill
      style={{
        backgroundColor: "#0d1117",
        justifyContent: "center",
        alignItems: "center",
        padding: 80,
      }}
    >
      <div
        style={{
          display: "flex",
          flexWrap: "wrap",
          justifyContent: "center",
          gap: 14,
          maxWidth: 900,
        }}
      >
        {words.map((word, i) => {
          const delay = i * 3;
          const opacity = interpolate(frame - delay, [0, 8], [0, 1], {
            extrapolateLeft: "clamp",
            extrapolateRight: "clamp",
          });
          const y = interpolate(frame - delay, [0, 8], [20, 0], {
            extrapolateLeft: "clamp",
            extrapolateRight: "clamp",
          });

          const isHighlight = ["build", "network", "stack", "scratch?"].includes(word);

          return (
            <span
              key={i}
              style={{
                fontSize: 58,
                fontWeight: 700,
                fontFamily: "'Inter', sans-serif",
                color: isHighlight ? "#00897B" : "#e6edf3",
                opacity,
                transform: `translateY(${y}px)`,
              }}
            >
              {word}
            </span>
          );
        })}
      </div>

      {/* Subtle cursor blink */}
      <div
        style={{
          position: "absolute",
          bottom: 120,
          left: "50%",
          transform: "translateX(-50%)",
          width: 3,
          height: 36,
          backgroundColor: "#00897B",
          opacity: Math.sin(frame * 0.3) > 0 ? 1 : 0,
        }}
      />
    </AbsoluteFill>
  );
};
