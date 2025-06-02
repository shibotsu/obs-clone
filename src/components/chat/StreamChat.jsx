import React, { useEffect, useRef, useState } from "react";
import { Button, Textarea } from "@fluentui/react-components";
import { Emoji16Regular, Send16Regular } from "@fluentui/react-icons";
import EmojiPicker from "emoji-picker-react";
import "./Chat.css";

export default function StreamChat() {
  const [messages, setMessages] = useState([]);
  const [input, setInput] = useState("");
  const [showEmojiPicker, setShowEmojiPicker] = useState(false);
  const ws = useRef(null);
  const inputRef = useRef(null);

  useEffect(() => {
    ws.current = new WebSocket("ws://localhost:6001/chat");

    ws.current.onmessage = (event) => {
      const msg = JSON.parse(event.data);
      setMessages((prev) => [...prev, msg]);
    };

    ws.current.onclose = () => {
      console.warn("Chat connection closed.");
    };

    return () => {
      ws.current.close();
    };
  }, []);

  const handleSend = () => {
    if (input.trim() === "") return;
    const message = { user: "You", text: input.trim() };
    ws.current.send(JSON.stringify(message));
    setMessages((prev) => [...prev, message]);
    setInput("");
  };

  const handleEmojiClick = (emojiData) => {
    const emoji = emojiData.emoji;
    setInput((prev) => prev + emoji);
    setShowEmojiPicker(false);
    inputRef.current?.focus();
  };

  return (
    <div
      style={{
        width: "300px",
        height: "100%",
        display: "flex",
        flexDirection: "column",
        flexShrink: 0,
        background: "white",
        position: "relative", // for emoji picker popup
      }}
    >
      <div
        style={{
          padding: "12px",
          borderBottom: "1px solid #eee",
          borderTop: "1px solid #eee",
          flexShrink: 0,
          fontWeight: "bold",
        }}
      >
        Stream Chat
      </div>

      <div
        style={{
          flex: 1,
          overflowY: "auto",
          padding: "12px",
        }}
      >
        {messages.map((msg, i) => (
          <div key={i} style={{ marginBottom: "8px" }}>
            <strong>{msg.user}</strong>: {msg.text}
          </div>
        ))}
      </div>

      <div
        style={{
          display: "flex",
          padding: "8px",
          borderTop: "1px solid #eee",
          flexShrink: 0,
          gap: "4px",
          maxHeight: "68px",
        }}
      >
        <Textarea
          ref={inputRef}
          type="text"
          placeholder="Send a message"
          className="custom-textarea"
          style={{
            width: "100%",
            padding: "8px",
            borderRadius: "4px",
            border: "1px solid #ccc",
            boxSizing: "border-box",
            maxHeight: "68px",
          }}
          value={input}
          onChange={(e) => setInput(e.currentTarget.value)}
          onKeyDown={(e) => {
            if (e.key === "Enter" && !e.shiftKey) {
              e.preventDefault();
              handleSend();
            }
          }}
        />

        <div
          style={{
            display: "flex",
            flexDirection: "column",
            gap: "4px",
            justifyContent: "flex-start",
          }}
        >
          <Button
            className="gradient-send-button"
            icon={<Send16Regular />}
            onClick={handleSend}
          />
          <Button
            icon={<Emoji16Regular />}
            onClick={() => setShowEmojiPicker((prev) => !prev)}
          />
        </div>
      </div>

      {showEmojiPicker && (
        <div
          style={{
            position: "absolute",
            bottom: "76px",
            right: "8px",
            zIndex: 10,
          }}
        >
          <EmojiPicker onEmojiClick={handleEmojiClick} />
        </div>
      )}
    </div>
  );
}
