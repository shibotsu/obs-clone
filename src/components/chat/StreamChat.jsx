import React from "react";
import { Button, Textarea } from "@fluentui/react-components";
import { Emoji16Regular, Send16Regular } from "@fluentui/react-icons";
import "./Chat.css";

export default function StreamChat() {
    return (
        <div
            style={{
                width: "300px",
                height: "100%",
                display: "flex",
                flexDirection: "column",
                flexShrink: 0,
                background: "white",
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
                {Array.from({ length: 50 }).map((_, i) => (
                    <div key={i} style={{ marginBottom: "8px" }}>
                        <strong>User{i + 1}</strong>: Hello world!
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
                    ></Button>
                    <Button icon={<Emoji16Regular />}></Button>
                </div>
            </div>
        </div>
    );
}
