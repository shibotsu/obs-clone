import React from "react";
import {
    ToggleButton,
    Menu,
    MenuTrigger,
    MenuPopover,
    MenuList,
    MenuItem,
    Tooltip,
} from "@fluentui/react-components";
import {
    Heart16Regular,
    Star16Regular,
    Share16Regular,
    MoreVertical16Regular,
    Person16Regular,
} from "@fluentui/react-icons";

export default function StreamHeader({ channel }) {
    return (
        <div
            style={{
                display: "flex",
                flexDirection: "column",
                padding: "20px",
                borderLeft: "4px solid",
                borderRight: "4px solid",
                borderImage: "linear-gradient(to bottom, #f7254d, #763a94) 1",
                width: "100%",
                boxSizing: "border-box",
            }}
        >
            <div
                style={{
                    display: "flex",
                    justifyContent: "space-between",
                    width: "100%",
                }}
            >
                <div style={{ display: "flex", alignItems: "center" }}>
                    <img
                        style={{ maxHeight: "100px", maxWidth: "100px" }}
                        src={channel.avatar}
                        alt=""
                        draggable={false}
                    />
                    <div
                        style={{
                            display: "flex",
                            flexDirection: "column",
                            alignItems: "flex-start",
                            justifyContent: "flex-start",
                            gap: "8px",
                        }}
                    >
                        <h2 style={{ margin: 0 }}>{channel.name}</h2>
                        {/* Extract Stream title from the channel prop in the future */}
                        <h3 style={{ margin: 0 }}>Stream title</h3>
                        <p style={{ margin: 0 }}>
                            Game / content that is streamed
                        </p>
                    </div>
                </div>
                <div style={{ diplay: "flex", flexDirection: "column" }}>
                    <div
                        style={{
                            display: "flex",
                            gap: "4px",
                        }}
                    >
                        <Tooltip
                            content="Follow"
                            positioning={{ position: "below", offset: 4 }}
                        >
                            <ToggleButton icon={<Heart16Regular />} />
                        </Tooltip>

                        <Tooltip
                            content="Share"
                            positioning={{ position: "below", offset: 4 }}
                        >
                            <ToggleButton icon={<Share16Regular />} />
                        </Tooltip>
                        <ToggleButton
                            icon={<Star16Regular />}
                            style={{
                                background:
                                    "linear-gradient(to right, #f7254d, #763a94)",
                                color: "white", // make text readable
                                padding: "8px 16px",
                                borderRadius: "6px",
                            }}
                        >
                            Subscribe
                        </ToggleButton>
                        <Menu>
                            <MenuTrigger disableButtonEnhancement>
                                <ToggleButton
                                    icon={<MoreVertical16Regular />}
                                />
                            </MenuTrigger>

                            <MenuPopover>
                                <MenuList>
                                    <MenuItem>Report Live Stream</MenuItem>
                                    <MenuItem>Report Something else</MenuItem>
                                </MenuList>
                            </MenuPopover>
                        </Menu>
                    </div>
                    <div
                        style={{
                            display: "flex",
                            justifyContent: "flex-end",
                            alignItems: "center",
                            gap: "8px",
                        }}
                    >
                        <Tooltip
                            content="Live viewers"
                            positioning={{ position: "below", offset: -10 }}
                        >
                            <div
                                style={{
                                    display: "flex",
                                    color: "red",
                                    justifyContent: "center",
                                    alignItems: "center",
                                    gap: "4px",
                                    cursor: "pointer", // optional for hover feedback
                                }}
                            >
                                <Person16Regular />
                                <p>11,768</p>
                            </div>
                        </Tooltip>
                        {/* Time elapsed from channel prop will go here */}
                        <Tooltip
                            content="Elapsed Time"
                            positioning={{ position: "below", offset: 4 }}
                        >
                            <div>1:47:19</div>
                        </Tooltip>
                    </div>
                </div>
            </div>
        </div>
    );
}
