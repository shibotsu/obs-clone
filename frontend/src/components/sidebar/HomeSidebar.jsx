import { useState } from "react";
import {
  DrawerBody,
  DrawerHeader,
  DrawerHeaderTitle,
  Drawer,
  Button,
  makeStyles,
  tokens,
  useRestoreFocusSource,
  useRestoreFocusTarget,
} from "@fluentui/react-components";
import { useQuery } from "@tanstack/react-query";
import { useAuth } from "../../context/AuthContext";
import { Dismiss24Regular, Navigation20Filled } from "@fluentui/react-icons";
import FollowingSidebarItem from "./FollowingSidebarItem";

const channels = [
  {
    id: 1,
    name: "Streamer1",
    avatar:
      "https://cdn3.iconfinder.com/data/icons/business-avatar-1/512/3_avatar-1024.png",
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

const useStyles = makeStyles({
  root: {
    overflow: "hidden",
    display: "inside",
    height: "48px",
    backgroundColor: "transparent",
  },
  content: {
    flex: "1",
    padding: "16px",
    display: "grid",
    justifyContent: "flex-start",
    alignItems: "flex-start",
    gridRowGap: tokens.spacingVerticalXXL,
    gridAutoRows: "max-content",
  },
  field: {
    display: "grid",
    gridRowGap: tokens.spacingVerticalS,
  },
  closeButton: {
    position: "relative",
    left: "200%",
  },
  sidebarButton: {
    backgroundColor: "transparent !important",
    color: "#000", // Change to desired visible color
    border: "none !important",
    boxShadow: "none !important",
    minWidth: "auto !important",
    // Override hover, active, and focus states:
    "&:hover": {
      backgroundColor: "transparent !important",
      color: "#000",
    },
    "&:active": {
      backgroundColor: "transparent !important",
      color: "#000",
    },
    "&:focus": {
      backgroundColor: "transparent !important",
      color: "#000",
    },
    // If the icon is inside an SVG, force its fill to be the same color:
    "& svg": {
      fill: "#000",
    },
  },
});

export const HomeSideBar = () => {
  const { token } = useAuth();
  const { data, isLoading } = useQuery({
    queryFn: () =>
      fetch("http://127.0.0.1:8000/api/following", {
        method: "GET",
        headers: {
          "Content-Type": "application/json",
          Authorization: `Bearer ${token}`,
        },
      }).then((res) => res.json()),
    queryKey: ["followingUsers"],
  });

  const following = data?.following || [];

  const styles = useStyles();

  const [isOpen, setIsOpen] = useState(false);

  // all Drawers need manual focus restoration attributes
  // unless (as in the case of some inline drawers, you do not want automatic focus restoration)
  const restoreFocusTargetAttributes = useRestoreFocusTarget();
  const restoreFocusSourceAttributes = useRestoreFocusSource();

  console.log(JSON.stringify(following));

  return (
    <div className={styles.root}>
      <Drawer
        {...restoreFocusSourceAttributes}
        type="overlay"
        separator
        open={isOpen}
        onOpenChange={(_, { open }) => setIsOpen(open)}
      >
        <DrawerHeader>
          <DrawerHeaderTitle
            action={
              <Button
                appearance="subtle"
                aria-label="Close"
                icon={<Dismiss24Regular />}
                className={styles.closeButton}
                onClick={() => setIsOpen(false)}
              />
            }
          >
            Following
          </DrawerHeaderTitle>
        </DrawerHeader>

        <DrawerBody>
          {following.map((channel) => (
            <FollowingSidebarItem
              id={channel.id}
              avatar={channel.profile_picture}
              name={channel.username}
            />
          ))}
        </DrawerBody>
      </Drawer>

      <div className={styles.content}>
        <Button
          {...restoreFocusTargetAttributes}
          appearance="primary"
          onClick={() => setIsOpen(!isOpen)}
          className={styles.sidebarButton}
        >
          <Navigation20Filled />
        </Button>
      </div>
    </div>
  );
};
