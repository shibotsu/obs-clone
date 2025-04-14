import React from "react";
import "../home/Home.css";
import { HomeSideBar } from "../../components/sidebar/HomeSidebar";
import CollapsedSidebar from "../../components/collapsed-sidebar/CollapsedSidebar";
import StreamHeader from "../../components/channel-description/StreamHeader";
import StreamDescription from "../../components/channel-description/StreamDescription";
import StreamChat from "../../components/chat/StreamChat";
import "./Watch.css";

// The channels object is to be removed from this component in the future
// and to be added in the layout section as it should be embedded in every page
// except the auth ones, idea is that collapsed sidebar should be a part of homesidebar
// component.

const channels = [
    {
        id: 1,
        name: "Streamer1",
        avatar: "https://cdn3.iconfinder.com/data/icons/business-avatar-1/512/3_avatar-1024.png",
    },
    { id: 2, name: "Streamer2", avatar: "" },
    { id: 3, name: "Streamer3", avatar: "" },
    { id: 4, name: "Streamer4", avatar: "" },
    { id: 5, name: "Streamer5", avatar: "" },
    { id: 6, name: "Streamer6", avatar: "" },
    { id: 7, name: "Streamer7", avatar: "" },
    { id: 8, name: "Streamer8", avatar: "" },
    { id: 9, name: "Streamer9", avatar: "" },
    { id: 10, name: "Streamer10", avatar: "" },
    { id: 11, name: "Streamer11", avatar: "" },
    { id: 12, name: "Streamer12", avatar: "" },
    { id: 13, name: "Streamer13", avatar: "" },
    { id: 14, name: "Streamer14", avatar: "" },
    { id: 15, name: "Streamer15", avatar: "" },
    { id: 16, name: "Streamer16", avatar: "" },
];

const thisChannel = {
    ...channels[0],
    description:
        "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?",
};

export default function WatchPage(user) {
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
                    <CollapsedSidebar channels={channels} />
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
                        src="" // Add stream as source or random video for preview.
                        controls
                        autoPlay
                        muted
                        style={{
                            aspectRatio: "16 / 9",
                            width: "100%",
                            objectFit: "cover",
                        }}
                    />
                    <StreamHeader channel={thisChannel} />
                    <StreamDescription channel={thisChannel} />
                </section>
                <StreamChat />
            </div>
        </div>
    );
}
