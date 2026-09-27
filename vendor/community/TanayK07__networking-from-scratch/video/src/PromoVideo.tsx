import { AbsoluteFill, Audio, Sequence, staticFile } from "remotion";
import { HookScene } from "./scenes/HookScene";
import { StatsScene } from "./scenes/StatsScene";
import { StackScene } from "./scenes/StackScene";
import { CodeScene } from "./scenes/CodeScene";
import { BuildListScene } from "./scenes/BuildListScene";
import { CapstoneScene } from "./scenes/CapstoneScene";
import { CTAScene } from "./scenes/CTAScene";

const FPS = 30;

export const PromoVideo: React.FC = () => {
  return (
    <AbsoluteFill style={{ backgroundColor: "#0d1117" }}>
      <Audio src={staticFile("audio/bg-music.mp3")} volume={1} />

      <Sequence from={0} durationInFrames={3 * FPS}>
        <HookScene />
      </Sequence>

      <Sequence from={3 * FPS} durationInFrames={4 * FPS}>
        <StatsScene />
      </Sequence>

      <Sequence from={7 * FPS} durationInFrames={8 * FPS}>
        <StackScene />
      </Sequence>

      <Sequence from={15 * FPS} durationInFrames={5 * FPS}>
        <CodeScene />
      </Sequence>

      <Sequence from={20 * FPS} durationInFrames={6 * FPS}>
        <BuildListScene />
      </Sequence>

      <Sequence from={26 * FPS} durationInFrames={4 * FPS}>
        <CapstoneScene />
      </Sequence>

      <Sequence from={30 * FPS} durationInFrames={5 * FPS}>
        <CTAScene />
      </Sequence>
    </AbsoluteFill>
  );
};
