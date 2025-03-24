import {
  Button,
  Input,
  Menu,
  MenuTrigger,
  MenuPopover,
  MenuList,
  MenuItem,
  Avatar,
  FluentProvider,
  webLightTheme,
} from "@fluentui/react-components";
import { useState } from "react";
import { Link } from "react-router-dom";
import "./header.css";

const Header = () => {
  const [isLoggedIn, setIsLoggedIn] = useState(true);

  return (
    <FluentProvider theme={webLightTheme}>
      <header>
        <Link to="/">Livestream app</Link>

        <Input placeholder="Search streams..." className="search-bar" />

        <div className="auth-container">
          {isLoggedIn ? (
            <Menu>
              <MenuTrigger disableButtonEnhancement style={{}}>
                <Avatar name="User" size={32} className="avatar" />
              </MenuTrigger>
              <MenuPopover>
                <MenuList>
                  <MenuItem>Profile</MenuItem>
                  <MenuItem>Settings</MenuItem>
                  <MenuItem onClick={() => setIsLoggedIn(false)}>
                    Log Out
                  </MenuItem>
                </MenuList>
              </MenuPopover>
            </Menu>
          ) : (
            <div>
              <Button style={{ marginRight: "0.5rem" }} appearance="primary">
                Log In
              </Button>
              <Button appearance="outline">Sign Up</Button>
            </div>
          )}
        </div>
      </header>
    </FluentProvider>
  );
};

export default Header;
