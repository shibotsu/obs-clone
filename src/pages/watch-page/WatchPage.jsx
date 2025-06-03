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

export default function WatchPage() {
  const { id } = useParams();
  const videoNode = useRef(null);
  const playerRef = useRef(null);

  const {
    data: channel,
    isLoading,
    isError,
  } = useQuery({
    queryKey: ["channel", id],
    queryFn: () =>
      fetch(`http://127.0.0.1:8000/api/channel/${id}/stream`).then((res) => {
        if (!res.ok) {
          throw new Error("Streamer not found");
        }
        return res.json();
      }),
  });

  useEffect(() => {
    if (!playerRef.current && videoNode.current) {
      playerRef.current = videojs(videoNode.current, {
        autoplay: true,
        controls: true,
        muted: true,
        preload: "auto",
        fluid: true,
        sources: [
          {
            src: `http://192.168.1.96:8080/hls/${id}.m3u8`,
            type: "application/x-mpegURL",
          },
        ],
      });
    }

    return () => {
      if (playerRef.current) {
        playerRef.current.dispose();
        playerRef.current = null;
      }
    };
  }, []);

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
          {channel && (
            <>
              <StreamHeader user={channel.user} channel={channel.channel} />
              <StreamDescription channel={channel.channel} />
            </>
          )}
        </section>

        <StreamChat />
      </div>
    </div>
  );
}
