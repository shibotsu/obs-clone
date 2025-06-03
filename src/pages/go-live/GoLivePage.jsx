import React, { useState } from "react";
import { useNavigate } from "react-router-dom";
import { Input, Button } from "@fluentui/react-components";
import { useAuth } from "../../context/AuthContext"; // adjust path as needed

export default function GoLivePage() {
  const [streamTitle, setStreamTitle] = useState("");
  const navigate = useNavigate();
  const { user, token } = useAuth(); // ✅ get from context

  const handleStartStreaming = async () => {
    if (!user?.id) {
      console.error("User ID not available");
      return;
    }

    try {
      const response = await fetch("http://127.0.0.1:8000/api/channel/start", {
        method: "POST",
        headers: {
          Authorization: `Bearer ${token}`,
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          stream_key: user.id, // ✅ from context
          title: streamTitle,
        }),
      });

      if (!response.ok) throw new Error("Failed to start stream");

      const data = await response.json();
      navigate(`/watch/${user.id}`); // ✅ use context user ID
    } catch (err) {
      console.error(err);
    }
  };

  return (
    <div style={{ padding: "40px", maxWidth: "500px", margin: "0 auto" }}>
      <h2>Go Live</h2>
      <Input
        label="Stream Title"
        placeholder="Enter a stream title"
        value={streamTitle}
        onChange={(e) => setStreamTitle(e.target.value)}
        style={{ marginBottom: "20px", width: "100%" }}
      />
      <Button
        appearance="primary"
        onClick={handleStartStreaming}
        disabled={!user?.id}
      >
        Start Streaming
      </Button>
    </div>
  );
}
