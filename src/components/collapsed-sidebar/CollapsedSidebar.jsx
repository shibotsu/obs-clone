import React from "react";
import FollowingSidebarItem from "../sidebar/FollowingSidebarItem";

export default function CollapsedSidebar({ channels }) {
    return (
        <div
            style={{
                display: "flex",
                flexDirection: "column",	
                alignItems: "center",
            }}
        >
            <div>
                {channels.map((channel) => {
                    return (
                        <FollowingSidebarItem
                            key={channel.id}
                            id={channel.id}
                            avatar={channel.avatar}
                        />
                    );
                })}
            </div>
        </div>
    );
}
