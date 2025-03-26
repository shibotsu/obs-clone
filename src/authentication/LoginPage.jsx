import { useState } from "react";
import { useNavigate } from "react-router-dom";
import {
  TextField,
  DefaultButton,
  Stack,
  PrimaryButton,
} from "@fluentui/react";
import "./Auth.css";
import { useAuth } from "./AuthContext";

const LoginPage = () => {
  const { login } = useAuth(); // will be used after implementing the backend
  const [password, setPassword] = useState("");
  const [usernameOrEmail, setUsernameOrEmail] = useState("");

  let navigate = useNavigate();

  const handleSubmit = () => {
    const isEmail = usernameOrEmail.includes("@");
    const requestData = {
      username: isEmail ? null : usernameOrEmail,
      email: isEmail ? usernameOrEmail : null,
      password: password,
    };

    const simulatedToken = "some-token";

    login(simulatedToken);

    navigate("/");
  };

  return (
    <div className="login-container">
      <Stack tokens={{ childrenGap: 10 }}>
        <TextField
          label="Username or Email"
          value={usernameOrEmail}
          onChange={(e, newValue) => setUsernameOrEmail(newValue || "")}
          required
        />
        <TextField
          label="Password"
          type="password"
          value={password}
          onChange={(e, newValue) => setPassword(newValue || "")}
          required
        />
        <PrimaryButton
          text="Login"
          onClick={handleSubmit}
          className="custom-primary-button"
        />
      </Stack>
    </div>
  );
};

export default LoginPage;
