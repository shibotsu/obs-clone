import React, { useRef, useEffect } from "react";
import "../home/Home.css";
import { useParams } from "react-router-dom";
import { useQuery } from "@tanstack/react-query";
import { HomeSideBar } from "../../components/sidebar/HomeSidebar";
import StreamHeader from "../../components/channel-description/StreamHeader";
import StreamDescription from "../../components/channel-description/StreamDescription";
import StreamChat from "../../components/chat/StreamChat";
import videojs from "video.js";
import "video.js/dist/video-js.css";
import "./Watch.css";
import { useAuth } from "../../context/AuthContext"; // ← ADDED

export default function WatchPage() {
  const { id } = useParams();
  const videoNode = useRef(null);
  const playerRef = useRef(null);
  const { token } = useAuth(); // ← ADDED

  const { data, isLoading, isError } = useQuery({
    queryKey: ["channel", id],
    queryFn: async () => {
      const res = await fetch(`http://157.230.16.67:8000/api/channel/${id}`, {
        headers: {
          "Content-Type": "application/json",
          Authorization: `Bearer ${token}`, // ← ADDED
        },
      });

      if (!res.ok) {
        throw new Error("Streamer not found");
      }

      return res.json(); // returns { channel, user }
    },
  });

  useEffect(() => {
    if (!data?.channel || !videoNode.current) return;

    const streamKey = data.channel.stream_key;
    const videoSrc = `http://157.230.16.67:8080/hls/${streamKey}.m3u8`;

    if (playerRef.current) {
      playerRef.current.src({ src: videoSrc, type: "application/x-mpegURL" });
      return;
    }

    playerRef.current = videojs(videoNode.current, {
      autoplay: true,
      controls: true,
      muted: true,
      preload: "auto",
      fluid: true,
      sources: [
        {
          src: videoSrc,
          type: "application/x-mpegURL",
        },
      ],
    });

    return () => {
      if (playerRef.current) {
        playerRef.current.dispose();
        playerRef.current = null;
      }
    };
  }, [data]);

  return (
    <div style={{ height: "calc(100vh - 65px)", overflow: "hidden" }}>
      <div style={{ width: "100%", display: "flex", height: "100%" }}>
        <div
          style={{
            backgroundColor: "white",
            height: "100vh",
            overflowY: "auto",
            flexShrink: 0,
          }}
        >
          <HomeSideBar className="sidebar-button" />
        </div>

        <section
          className="scroll-hidden"
          style={{
            backgroundColor: "#EEF5DB",
            display: "flex",
            flex: "3",
            flexDirection: "column",
            gap: "4px",
            height: "100%",
            overflowY: "auto",
          }}
        >
          <div data-vjs-player>
            <video
              ref={videoNode}
              className="video-js vjs-default-skin"
              style={{
                aspectRatio: "16 / 9",
                width: "100%",
                objectFit: "cover",
              }}
            ></video>
          </div>

          {isLoading && <p>Loading stream info...</p>}
          {isError && <p>Streamer not found.</p>}
          {data && (
            <>
              <StreamHeader channel={data.channel} user={data.user} />
              <StreamDescription channel={data.channel} user={data.user} />
            </>
          )}
        </section>

        <StreamChat />
      </div>
    </div>
  );
}
