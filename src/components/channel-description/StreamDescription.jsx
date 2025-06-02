import React from "react";

export default function StreamDescription({ channel }) {
    return (
        <div
            style={{
                display: "flex",
                flexDirection: "column",
                padding: "24px",
            }}
        >
            {channel.description}
        </div>
    );
}
