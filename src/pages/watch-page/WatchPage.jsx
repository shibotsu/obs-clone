import React from "react";
import "../home/Home.css";
import { useParams } from "react-router-dom";
import { useQuery } from "@tanstack/react-query";
import { HomeSideBar } from "../../components/sidebar/HomeSidebar";
import StreamHeader from "../../components/channel-description/StreamHeader";
import StreamDescription from "../../components/channel-description/StreamDescription";
import StreamChat from "../../components/chat/StreamChat";
import "./Watch.css";

export default function WatchPage() {
  const { id } = useParams();

  const {
    data: channel,
    isLoading,
    isError,
  } = useQuery({
    queryKey: ["channel", id],
    queryFn: () =>
      fetch(`http://127.0.0.1:8000/api/profile/${id}`).then((res) => {
        if (!res.ok) {
          throw new Error("Streamer not found");
        }
        return res.json();
      }),
  });

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
          <video
            src="" // Replace with actual stream source or preview
            controls
            autoPlay
            muted
            style={{
              aspectRatio: "16 / 9",
              width: "100%",
              objectFit: "cover",
            }}
          />
          {isLoading && <p>Loading stream info...</p>}
          {isError && <p>Streamer not found.</p>}
          {channel && (
            <>
              <StreamHeader channel={channel} />
              <StreamDescription channel={channel} />
            </>
          )}
        </section>

        <StreamChat />
      </div>
    </div>
  );
}
