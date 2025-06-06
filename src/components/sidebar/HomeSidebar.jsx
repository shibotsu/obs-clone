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

  console.log(token);

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
        {token && (
          <Button
            {...restoreFocusTargetAttributes}
            appearance="primary"
            onClick={() => setIsOpen(!isOpen)}
            className={styles.sidebarButton}
          >
            <Navigation20Filled />
          </Button>
        )}
      </div>
    </div>
  );
};
